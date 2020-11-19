/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "catapult/local/recovery/RepairState.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/io/IndexFile.h"
#include "catapult/io/PodIoUtils.h"
#include "catapult/subscribers/SubscriberOperationTypes.h"
#include "catapult/utils/Casting.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/other/mocks/MockStateChangeSubscriber.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS RepairStateTests

	namespace {
		// region TestContext

		std::string GetMessageFilename(uint64_t value) {
			std::ostringstream out;
			out << utils::HexFormat(value) << ".dat";
			return out.str();
		}

		class TestContext {
		private:
			static constexpr auto Queue_Name = "foo";

		public:
			TestContext()
					: m_dataDirectory(m_tempDir.name())
					, m_catapultCache({}) {
				std::filesystem::create_directories(m_dataDirectory.spoolDir(Queue_Name).path());
			}

		public:
			void repair() {
				return RepairState(m_dataDirectory.spoolDir(Queue_Name), m_catapultCache, m_registeredSubscriber, m_repairSubscriber);
			}

		public:
			const auto& registeredSubscriber() const {
				return m_registeredSubscriber;
			}

			const auto& repairSubscriber() const {
				return m_repairSubscriber;
			}

		public:
			bool exists(const std::string& indexName) const {
				return std::filesystem::exists(m_dataDirectory.spoolDir(Queue_Name).file(indexName));
			}

			bool messageExists(uint64_t value) const {
				return std::filesystem::exists(m_dataDirectory.spoolDir(Queue_Name).file(GetMessageFilename(value)));
			}

			uint64_t readIndex(const std::string& indexName) const {
				return io::IndexFile(m_dataDirectory.spoolDir(Queue_Name).file(indexName)).get();
			}

			void setIndex(const std::string& indexName, uint64_t indexValue) {
				io::IndexFile(m_dataDirectory.spoolDir(Queue_Name).file(indexName)).set(indexValue);
			}

			void removeIndex(const std::string& indexName) {
				std::filesystem::remove(m_dataDirectory.spoolDir(Queue_Name).file(indexName));
			}

			void writeMessages(uint64_t startValue, uint64_t endValue) {
				for (auto value = startValue; value <= endValue; ++value) {
					// write chain score message
					auto messageFilePath = m_dataDirectory.spoolDir(Queue_Name).file(GetMessageFilename(value));
					io::RawFile messageFile(messageFilePath, io::OpenMode::Read_Write);
					io::Write8(messageFile, utils::to_underlying_type(subscribers::StateChangeOperationType::Score_Change));
					io::Write64(messageFile, 0);
					io::Write64(messageFile, value);
				}
			}

		private:
			test::TempDirectoryGuard m_tempDir;
			config::CatapultDataDirectory m_dataDirectory;
			cache::CatapultCache m_catapultCache;
			mocks::MockStateChangeSubscriber m_registeredSubscriber;
			mocks::MockStateChangeSubscriber m_repairSubscriber;
		};

		// endregion

		// region traits

		// (index filenames are numbered from lowest to highest)

		struct ServerTraits {
			static constexpr auto Index_Filename1 = "index_server_r.dat";
			static constexpr auto Index_Filename2 = "index.dat";
			static constexpr auto Index_Filename3 = "index_server.dat";
		};

		struct BrokerTraits {
			static constexpr auto Index_Filename1 = "index_broker_r.dat";
			static constexpr auto Index_Filename2 = "index.dat";
			static constexpr auto Index_Filename3 = "index_server.dat";
		};

#define REPAIR_STATE_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Server) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ServerTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Broker) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<BrokerTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

		// endregion
	}

	// region tests

	REPAIR_STATE_TEST(RepairSucceedsWhenAllSubscribersAreCaughtUp) {
		// Arrange:
		TestContext context;
		context.setIndex(TTraits::Index_Filename1, 111);
		context.setIndex(TTraits::Index_Filename2, 111);
		context.setIndex(TTraits::Index_Filename3, 111);

		// Act:
		context.repair();

		// Assert:
		EXPECT_EQ(111u, context.readIndex(TTraits::Index_Filename1));
		EXPECT_EQ(111u, context.readIndex(TTraits::Index_Filename2));
		EXPECT_EQ(111u, context.readIndex(TTraits::Index_Filename3));

		EXPECT_EQ(0u, context.registeredSubscriber().numScoreChanges());
		EXPECT_EQ(0u, context.repairSubscriber().numScoreChanges());
	}

	REPAIR_STATE_TEST(RepairSucceedsWhenRegisteredSubscriberIsBehind) {
		// Arrange:
		TestContext context;
		context.setIndex(TTraits::Index_Filename1, 105);
		context.setIndex(TTraits::Index_Filename2, 111);
		context.setIndex(TTraits::Index_Filename3, 111);
		context.writeMessages(105, 111);

		// Act:
		context.repair();

		// Assert:
		EXPECT_EQ(111u, context.readIndex(TTraits::Index_Filename1));
		EXPECT_EQ(111u, context.readIndex(TTraits::Index_Filename2));
		EXPECT_EQ(111u, context.readIndex(TTraits::Index_Filename3));

		EXPECT_FALSE(context.messageExists(110));
		EXPECT_TRUE(context.messageExists(111));

		EXPECT_EQ(6u, context.registeredSubscriber().numScoreChanges());
		EXPECT_EQ(0u, context.repairSubscriber().numScoreChanges());
	}

	REPAIR_STATE_TEST(RepairSucceedsWhenRepairSubscriberIsBehind) {
		// Arrange:
		TestContext context;
		context.setIndex(TTraits::Index_Filename1, 105);
		context.setIndex(TTraits::Index_Filename2, 105);
		context.setIndex(TTraits::Index_Filename3, 111);
		context.writeMessages(105, 111);

		// Act:
		context.repair();

		// Assert:
		EXPECT_EQ(111u, context.readIndex(TTraits::Index_Filename1));
		EXPECT_EQ(111u, context.readIndex(TTraits::Index_Filename2));
		EXPECT_EQ(111u, context.readIndex(TTraits::Index_Filename3));

		EXPECT_FALSE(context.messageExists(110));
		EXPECT_TRUE(context.messageExists(111));

		EXPECT_EQ(0u, context.registeredSubscriber().numScoreChanges());
		EXPECT_EQ(6u, context.repairSubscriber().numScoreChanges());
	}

	REPAIR_STATE_TEST(RepairSucceedsWhenRegisteredAndRepairSubscribersAreBehindDifferentAmounts) {
		// Arrange:
		TestContext context;
		context.setIndex(TTraits::Index_Filename1, 105);
		context.setIndex(TTraits::Index_Filename2, 107);
		context.setIndex(TTraits::Index_Filename3, 111);
		context.writeMessages(105, 111);

		// Act:
		context.repair();

		// Assert:
		EXPECT_EQ(111u, context.readIndex(TTraits::Index_Filename1));
		EXPECT_EQ(111u, context.readIndex(TTraits::Index_Filename2));
		EXPECT_EQ(111u, context.readIndex(TTraits::Index_Filename3));

		EXPECT_FALSE(context.messageExists(110));
		EXPECT_TRUE(context.messageExists(111));

		EXPECT_EQ(2u, context.registeredSubscriber().numScoreChanges());
		EXPECT_EQ(4u, context.repairSubscriber().numScoreChanges());
	}

	REPAIR_STATE_TEST(RepairSucceedsWhenOnlyFinalIndexReaderFileIsPresent) {
		// Arrange:
		TestContext context;
		context.setIndex(TTraits::Index_Filename1, 111);
		context.removeIndex(TTraits::Index_Filename2);
		context.removeIndex(TTraits::Index_Filename3);

		// Act:
		context.repair();

		// Assert: Index_Filename2 gets created by repair
		EXPECT_EQ(0u, context.readIndex(TTraits::Index_Filename1));
		EXPECT_EQ(0u, context.readIndex(TTraits::Index_Filename2));
		EXPECT_FALSE(context.exists(TTraits::Index_Filename3));

		EXPECT_EQ(0u, context.registeredSubscriber().numScoreChanges());
		EXPECT_EQ(0u, context.repairSubscriber().numScoreChanges());
	}

	// endregion
}}
