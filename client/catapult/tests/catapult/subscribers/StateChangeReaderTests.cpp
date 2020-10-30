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

#include "catapult/subscribers/StateChangeReader.h"
#include "catapult/subscribers/SubscriberOperationTypes.h"
#include "catapult/preprocessor.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/mocks/MockMemoryStream.h"
#include "tests/test/other/mocks/MockStateChangeSubscriber.h"
#include "tests/TestHarness.h"

namespace catapult { namespace subscribers {

#define TEST_CLASS StateChangeReaderTests

	namespace {
		std::vector<uint8_t> CreateSerializedDataBuffer(StateChangeOperationType operationType, const std::vector<uint64_t>& values) {
			std::vector<uint8_t> buffer(1 + values.size() * sizeof(uint64_t));
			buffer[0] = utils::to_underlying_type(operationType);
			std::memcpy(&buffer[1], values.data(), buffer.size() - 1);
			return buffer;
		}

		// region MockCacheChangesStorageReader / ReadValueAt

		// simulate CacheChangesStorage by reading single uint32_t value in loadAll and inserting it into Added container (as sentinel)
		template<size_t CacheId>
		class MockCacheChangesStorageReader : public cache::CacheChangesStorage {
		public:
			size_t id() const override {
				return CacheId;
			}

		public:
			void saveAll(const cache::CacheChanges&, io::OutputStream&) const override {
				CATAPULT_THROW_INVALID_ARGUMENT("saveAll - not supported in mock");
			}

			std::unique_ptr<const cache::MemoryCacheChanges> loadAll(io::InputStream& input) const override {
				auto pMemoryCacheChanges = std::make_unique<cache::MemoryCacheChangesT<uint32_t>>();
				pMemoryCacheChanges->Added.push_back(io::Read32(input));
				return PORTABLE_MOVE(pMemoryCacheChanges);
			}

			void apply(const cache::CacheChanges&) const override {
				CATAPULT_THROW_INVALID_ARGUMENT("apply - not supported in mock");
			}
		};

		// cruft required for calling CacheChanges::sub
		template<size_t CacheId>
		struct MockCacheTraits {
			struct CacheDeltaType {
				std::unordered_set<const uint32_t*> addedElements() const {
					return {};
				}
			};
			using CacheValueType = uint32_t;

			static constexpr auto Id = CacheId;
		};

		// reads single uint32_t value that was written to Added container and extracted into addedElements
		template<size_t CacheId>
		uint32_t ReadValueAt(const StateChangeInfo& stateChangeInfo) {
			const auto& addedElements = stateChangeInfo.CacheChanges.sub<MockCacheTraits<CacheId>>().addedElements();
			EXPECT_EQ(1u, addedElements.size()) << CacheId;
			if (addedElements.empty())
				CATAPULT_THROW_INVALID_ARGUMENT_1("addedElements is empty", CacheId);

			return **addedElements.cbegin();
		}

		// endregion
	}

	TEST(TEST_CLASS, CanReadSingleScoreChange) {
		// Arrange:
		auto values = test::GenerateRandomDataVector<uint64_t>(2);
		auto buffer = CreateSerializedDataBuffer(StateChangeOperationType::Score_Change, values);

		mocks::MockMemoryStream stream(buffer);
		mocks::MockStateChangeSubscriber subscriber;

		// Act:
		ReadNextStateChange(stream, {}, subscriber);

		// Assert:
		ASSERT_EQ(1u, subscriber.numScoreChanges());
		EXPECT_EQ(0u, subscriber.numStateChanges());

		EXPECT_EQ(model::ChainScore(values[0], values[1]), subscriber.lastChainScore());
	}

	TEST(TEST_CLASS, CanReadSingleStateChange) {
		// Arrange:
		auto values = test::GenerateRandomDataVector<uint64_t>(3);
		auto buffer = CreateSerializedDataBuffer(StateChangeOperationType::State_Change, values);

		// - simulate two cache changes storages
		mocks::MockMemoryStream stream(buffer);
		CacheChangesStorages cacheChangesStorages;
		cacheChangesStorages.emplace_back(std::make_unique<MockCacheChangesStorageReader<3>>());
		cacheChangesStorages.emplace_back(std::make_unique<MockCacheChangesStorageReader<7>>());
		mocks::MockStateChangeSubscriber subscriber;

		// Act:
		ReadNextStateChange(stream, cacheChangesStorages, subscriber);

		// Assert:
		EXPECT_EQ(0u, subscriber.numScoreChanges());
		ASSERT_EQ(1u, subscriber.numStateChanges());

		const auto& capturedStateChangeInfo = subscriber.lastStateChangeInfo();
		EXPECT_EQ(model::ChainScore::Delta(static_cast<int64_t>(values[0])), capturedStateChangeInfo.ScoreDelta);
		EXPECT_EQ(Height(values[1]), capturedStateChangeInfo.Height);

		// - each cache changes storage extracted single uint32_t value
		EXPECT_EQ(values[2] & 0xFFFF'FFFF, ReadValueAt<3>(capturedStateChangeInfo));
		EXPECT_EQ(values[2] >> 32, ReadValueAt<7>(capturedStateChangeInfo));
	}

	TEST(TEST_CLASS, CannotReadSingleUnknownOperationType) {
		// Arrange:
		auto values = test::GenerateRandomDataVector<uint64_t>(2);
		auto buffer = CreateSerializedDataBuffer(static_cast<StateChangeOperationType>(123), values);

		mocks::MockMemoryStream stream(buffer);
		mocks::MockStateChangeSubscriber subscriber;

		// Act + Assert:
		EXPECT_THROW(ReadNextStateChange(stream, {}, subscriber), catapult_invalid_argument);
	}
}}
