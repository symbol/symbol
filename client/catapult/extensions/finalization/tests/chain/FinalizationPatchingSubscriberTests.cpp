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

#include "finalization/src/chain/FinalizationPatchingSubscriber.h"
#include "finalization/src/io/PrevoteChainStorage.h"
#include "finalization/tests/test/FinalizationMessageTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockMemoryBlockStorage.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

#define TEST_CLASS FinalizationPatchingSubscriberTests

	namespace {
		// region MockPrevoteChainStorage

		class MockPrevoteChainStorage : public io::PrevoteChainStorage {
		public:
			bool contains(const model::FinalizationRound& round, const model::HeightHashPair& heightHashPair) const override {
				m_containsRounds.push_back(round);
				return m_containedHeightHashPair == heightHashPair;
			}

			model::BlockRange load(const model::FinalizationRound& round, Height maxHeight) const override {
				m_loadRounds.push_back(round);
				return test::CreateBlockEntityRange(maxHeight.unwrap());
			}

			void save(const io::BlockStorageView&, const io::PrevoteChainDescriptor&) override {
				CATAPULT_THROW_RUNTIME_ERROR("save - not supported in mock");
			}

			void remove(const model::FinalizationRound& round) override {
				m_removeRounds.push_back(round);
			}

		public:
			const auto& containsRounds() const {
				return m_containsRounds;
			}

			const auto& loadRounds() const {
				return m_loadRounds;
			}

			const auto& removeRounds() const {
				return m_removeRounds;
			}

		public:
			void setContained(const model::HeightHashPair& heightHashPair) {
				m_containedHeightHashPair = heightHashPair;
			}

		private:
			model::HeightHashPair m_containedHeightHashPair;

			mutable std::vector<model::FinalizationRound> m_containsRounds;
			mutable std::vector<model::FinalizationRound> m_loadRounds;
			std::vector<model::FinalizationRound> m_removeRounds;
		};

		// endregion

		// region TestContext

		class TestContext {
		public:
			explicit TestContext(uint32_t numBlocks)
					: m_pBlockStorageCache(mocks::CreateMemoryBlockStorageCache(numBlocks))
					, m_subscriber(m_prevoteChainStorage, *m_pBlockStorageCache, [this](auto&& blockRange) {
						m_blockRangeSizes.push_back(blockRange.size());
					})
			{}

		public:
			auto& prevoteChainStorage() {
				return m_prevoteChainStorage;
			}

			auto blockStorageHash(Height height) {
				return m_pBlockStorageCache->view().loadBlockElement(height)->EntityHash;
			}

			auto& blockRangeSizes() {
				return m_blockRangeSizes;
			}

			auto& subscriber() {
				return m_subscriber;
			}

		private:
			MockPrevoteChainStorage m_prevoteChainStorage;
			std::unique_ptr<io::BlockStorageCache> m_pBlockStorageCache;
			std::vector<size_t> m_blockRangeSizes;

			FinalizationPatchingSubscriber m_subscriber;
		};

		// endregion

		// region test utils

		void AssertChainIsNotLoadedFromBackups(TestContext& context, const model::FinalizationRound& round, bool isInBlockStorage) {
			// if block storage already contains finalized block, contains check in backups is bypassed
			if (isInBlockStorage)
				EXPECT_EQ(std::vector<model::FinalizationRound>(), context.prevoteChainStorage().containsRounds());
			else
				EXPECT_EQ(std::vector<model::FinalizationRound>({ round }), context.prevoteChainStorage().containsRounds());

			EXPECT_EQ(std::vector<model::FinalizationRound>(), context.prevoteChainStorage().loadRounds());
			EXPECT_EQ(std::vector<model::FinalizationRound>({ round }), context.prevoteChainStorage().removeRounds());

			EXPECT_EQ(std::vector<size_t>(), context.blockRangeSizes());
		}

		void AssertChainIsLoadedFromBackups(TestContext& context, const model::FinalizationRound& round, Height height) {
			EXPECT_EQ(std::vector<model::FinalizationRound>({ round }), context.prevoteChainStorage().containsRounds());
			EXPECT_EQ(std::vector<model::FinalizationRound>({ round }), context.prevoteChainStorage().loadRounds());
			EXPECT_EQ(std::vector<model::FinalizationRound>({ round }), context.prevoteChainStorage().removeRounds());

			EXPECT_EQ(std::vector<size_t>({ height.unwrap() }), context.blockRangeSizes());
		}

		// endregion
	}

	// region tests

	TEST(TEST_CLASS, PrevoteChainIsNotLoadedWhenFinalizedBlockIsInNeitherStorageNorPrevoteBackups) {
		// Arrange:
		TestContext context(10);
		context.prevoteChainStorage().setContained({ Height(7), test::GenerateRandomByteArray<Hash256>() });

		// Act:
		auto round = test::CreateFinalizationRound(3, 11);
		context.subscriber().notifyFinalizedBlock(round, Height(7), test::GenerateRandomByteArray<Hash256>());

		// Assert:
		AssertChainIsNotLoadedFromBackups(context, round, false);
	}

	TEST(TEST_CLASS, PrevoteChainIsNotLoadedWhenFinalizedBlockIsInStorageButNotPrevoteBackups) {
		// Arrange:
		TestContext context(10);
		context.prevoteChainStorage().setContained({ Height(7), test::GenerateRandomByteArray<Hash256>() });

		// Act:
		auto round = test::CreateFinalizationRound(3, 11);
		context.subscriber().notifyFinalizedBlock(round, Height(7), context.blockStorageHash(Height(7)));

		// Assert:
		AssertChainIsNotLoadedFromBackups(context, round, true);
	}

	TEST(TEST_CLASS, PrevoteChainIsLoadedWhenFinalizedBlockIsInPrevoteBackupsButNotStorage) {
		// Arrange:
		auto hash = test::GenerateRandomByteArray<Hash256>();
		TestContext context(10);
		context.prevoteChainStorage().setContained({ Height(7), hash });

		// Act:
		auto round = test::CreateFinalizationRound(3, 11);
		context.subscriber().notifyFinalizedBlock(round, Height(7), hash);

		// Assert:
		AssertChainIsLoadedFromBackups(context, round, Height(7));
	}

	TEST(TEST_CLASS, PrevoteChainIsLoadedWhenFinalizedBlockIsInPrevoteBackupsButNotStorageAndAheadOfStorage) {
		// Arrange:
		auto hash = test::GenerateRandomByteArray<Hash256>();
		TestContext context(10);
		context.prevoteChainStorage().setContained({ Height(12), hash });

		// Act:
		auto round = test::CreateFinalizationRound(3, 11);
		context.subscriber().notifyFinalizedBlock(round, Height(12), hash);

		// Assert:
		AssertChainIsLoadedFromBackups(context, round, Height(12));
	}

	TEST(TEST_CLASS, PrevoteChainIsNotLoadedWhenFinalizedBlockIsInBothStorageAndPrevoteBackups) {
		// Arrange:
		TestContext context(10);
		context.prevoteChainStorage().setContained({ Height(7), context.blockStorageHash(Height(7)) });

		// Act:
		auto round = test::CreateFinalizationRound(3, 11);
		context.subscriber().notifyFinalizedBlock(round, Height(7), context.blockStorageHash(Height(7)));

		// Assert:
		AssertChainIsNotLoadedFromBackups(context, round, true);
	}

	// endregion
}}
