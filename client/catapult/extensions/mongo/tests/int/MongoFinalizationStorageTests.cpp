/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "mongo/src/MongoFinalizationStorage.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoTestUtils.h"
#include "tests/TestHarness.h"
#include <mongocxx/instance.hpp>
#include <unordered_map>

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo {

#define TEST_CLASS MongoFinalizationStorageTests

	namespace {
		constexpr size_t Num_Finalized_Blocks = 10;
		constexpr auto Collection_Name = "finalizedBlocks";

		// region test utils

		struct FinalizedBlockDescriptor {
			catapult::Height Height;
			Hash256 Hash;
			FinalizationPoint Point;
		};

		using FinalizedBlocksMap = std::unordered_map<Height, FinalizedBlockDescriptor, utils::BaseValueHasher<Height>>;

		auto ResetDatabaseAndCreateMongoContext(thread::IoThreadPool& pool) {
			test::ResetDatabase(test::DatabaseName());
			return test::CreateDefaultMongoStorageContext(test::DatabaseName(), pool);
		}

		auto CreateFinalizedBlockDescriptors(size_t count) {
			std::vector<FinalizedBlockDescriptor> descriptors;
			for (auto i = 1u; i <= count; ++i)
				descriptors.push_back({ Height(i * i), test::GenerateRandomByteArray<Hash256>(), FinalizationPoint(i) });

			return descriptors;
		}

		void AssertFinalizedBlocks(const FinalizedBlocksMap& expectedDescriptors) {
			auto connection = test::CreateDbConnection();
			auto database = connection[test::DatabaseName()];
			auto collection = database[Collection_Name];

			// Assert: check collection size
			EXPECT_EQ(expectedDescriptors.size(), static_cast<size_t>(collection.count_documents({})));

			auto txCursor = collection.find({});
			for (const auto& view : txCursor) {
				auto blockView = view["block"].get_document().view();

				auto dbHeight = Height(test::GetUint64(blockView, "height"));
				auto expectedIter = expectedDescriptors.find(dbHeight);
				ASSERT_TRUE(expectedDescriptors.cend() != expectedIter);

				const auto& descriptor = expectedIter->second;
				EXPECT_EQ(descriptor.Height, dbHeight);
				EXPECT_EQ(descriptor.Hash, test::GetHashValue(blockView, "hash"));
				EXPECT_EQ(descriptor.Point, FinalizationPoint(test::GetUint64(blockView, "finalizationPoint")));
			}
		}

		// endregion

		// region FinalizationSubscriberContext

		class FinalizationSubscriberContext {
		public:
			explicit FinalizationSubscriberContext(size_t numFinalizationes)
					: m_pPool(test::CreateStartedIoThreadPool(test::Num_Default_Mongo_Test_Pool_Threads))
					, m_pMongoContext(ResetDatabaseAndCreateMongoContext(*m_pPool))
					, m_pSubscriber(CreateMongoFinalizationStorage(*m_pMongoContext))
					, m_descriptors(CreateFinalizedBlockDescriptors(numFinalizationes))
			{}

		public:
			const std::vector<FinalizedBlockDescriptor>& descriptors() const {
				return m_descriptors;
			}

			FinalizedBlocksMap toMap() const {
				FinalizedBlocksMap map;
				for (const auto& descriptor : m_descriptors)
					map.emplace(descriptor.Height, descriptor);

				return map;
			}

		public:
			void saveFinalizedBlock(const FinalizedBlockDescriptor& descriptor) {
				m_pSubscriber->notifyFinalizedBlock(descriptor.Height, descriptor.Hash, descriptor.Point);
			}

		private:
			std::unique_ptr<thread::IoThreadPool> m_pPool;
			std::unique_ptr<MongoStorageContext> m_pMongoContext;
			std::unique_ptr<subscribers::FinalizationSubscriber> m_pSubscriber;
			std::vector<FinalizedBlockDescriptor> m_descriptors;
		};

		// endregion
	}

	// region notifyFinalizedBlock

	TEST(TEST_CLASS, CanSaveSingleFinalizedBlock) {
		// Arrange:
		FinalizationSubscriberContext context(1);

		// Sanity:
		test::AssertCollectionSize(Collection_Name, 0);

		// Act:
		context.saveFinalizedBlock(context.descriptors().back());

		// Assert:
		AssertFinalizedBlocks(context.toMap());
	}

	TEST(TEST_CLASS, CanSaveMultipleFinalizedBlocks) {
		// Arrange:
		FinalizationSubscriberContext context(Num_Finalized_Blocks);

		// Sanity:
		test::AssertCollectionSize(Collection_Name, 0);

		// Act:
		for (const auto& descriptor : context.descriptors())
			context.saveFinalizedBlock(descriptor);

		// Assert:
		AssertFinalizedBlocks(context.toMap());
	}

	// endregion
}}
