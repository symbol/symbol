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

#include "mongo/src/storages/MongoBlockDifficultyCacheStorage.h"
#include "mongo/src/MongoBlockStorage.h"
#include "mongo/src/MongoDatabase.h"
#include "catapult/cache_core/BlockDifficultyCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "mongo/tests/test/MongoTestUtils.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo { namespace storages {

#define TEST_CLASS MongoBlockDifficultyCacheStorageTests

	namespace {
		constexpr uint64_t Multiple_Blocks_Count = 10;
		constexpr uint64_t Difficulty_History_Size = 100;

		class TestContext final : public test::PrepareDatabaseMixin {
		public:
			explicit TestContext(size_t topHeight)
					: m_pMongoContext(test::CreateDefaultMongoStorageContext(test::DatabaseName()))
					, m_transactionRegistry(test::CreateDefaultMongoTransactionRegistry())
					, m_pCacheStorage(CreateMongoBlockDifficultyCacheStorage(
							m_pMongoContext->createDatabaseConnection(),
							Difficulty_History_Size))
					, m_pDbStorage(CreateMongoBlockStorage(*m_pMongoContext, m_transactionRegistry)) {
				for (auto i = 1u; i <= topHeight; ++i) {
					auto transactions = test::GenerateRandomTransactions(10);
					m_blocks.push_back(test::GenerateRandomBlockWithTransactions(test::MakeConst(transactions)));
					m_blocks.back()->Height = Height(i);
					m_blockElements.emplace_back(test::BlockToBlockElement(*m_blocks.back(), test::GenerateRandomData<Hash256_Size>()));
				}

				saveBlocks();
			}

		public:
			ExternalCacheStorage& cacheStorage() {
				return *m_pCacheStorage;
			}

			void saveBlocks() {
				for (const auto& blockElement : m_blockElements)
					m_pDbStorage->saveBlock(blockElement);
			}

			const std::vector<model::BlockElement>& elements() {
				return m_blockElements;
			}

		private:
			std::vector<std::unique_ptr<model::Block>> m_blocks;
			std::vector<model::BlockElement> m_blockElements;
			std::unique_ptr<MongoStorageContext> m_pMongoContext;
			mongo::MongoTransactionRegistry m_transactionRegistry;
			std::unique_ptr<ExternalCacheStorage> m_pCacheStorage;
			std::unique_ptr<io::LightBlockStorage> m_pDbStorage;
		};

		auto ToBlockDifficultyInfo(const model::Block& block) {
			return state::BlockDifficultyInfo(block.Height, block.Timestamp, block.Difficulty);
		}

		template<typename TIter>
		void AssertDifficultyInfos(TIter startIter, TIter endIter, const cache::BlockDifficultyCacheView& infos) {
			auto numIterations = 0u;
			for (; endIter != startIter; ++startIter) {
				const auto& blockElement = *startIter;
				EXPECT_TRUE(infos.contains(ToBlockDifficultyInfo(blockElement.Block))) << "at block " << blockElement.Block.Height;
				++numIterations;
			}

			EXPECT_EQ(infos.size(), numIterations);
		}
	}

	// region saveDelta

	TEST(TEST_CLASS, SaveDeltaIsNoOp) {
		// Arrange:
		TestContext context(Multiple_Blocks_Count);
		auto chainConfig = model::BlockChainConfiguration::Uninitialized();
		chainConfig.MaxDifficultyBlocks = Difficulty_History_Size;
		auto cache = test::CreateEmptyCatapultCache(chainConfig);
		context.cacheStorage().loadAll(cache, Height(Multiple_Blocks_Count));

		// Sanity:
		test::AssertCollectionSize("blocks", Multiple_Blocks_Count);

		auto delta = cache.createDelta();
		auto& blockDifficultyCacheDelta = delta.sub<cache::BlockDifficultyCache>();
		blockDifficultyCacheDelta.insert(Height(Multiple_Blocks_Count + 1), Timestamp(123), Difficulty());

		// Act:
		context.cacheStorage().saveDelta(delta);

		// Assert:
		test::AssertCollectionSize("blocks", Multiple_Blocks_Count);
	}

	// endregion

	// region loadAll

	TEST(TEST_CLASS, CanLoadDifficulties) {
		// Arrange:
		TestContext context(Multiple_Blocks_Count);
		auto chainConfig = model::BlockChainConfiguration::Uninitialized();
		chainConfig.MaxDifficultyBlocks = Difficulty_History_Size;
		auto cache = test::CreateEmptyCatapultCache(chainConfig);

		// Act: load 6 difficulties
		context.cacheStorage().loadAll(cache, Height(6));

		// Assert:
		auto view = cache.createView();
		const auto& difficultyView = view.sub<cache::BlockDifficultyCache>();
		ASSERT_EQ(6u, difficultyView.size());
		auto start = context.elements().cbegin();
		auto end = context.elements().cbegin();
		std::advance(end, 6);
		AssertDifficultyInfos(start, end, difficultyView);
	}

	TEST(TEST_CLASS, LoadAllLoadsAtMostAvailableDifficulties) {
		// Arrange:
		TestContext context(Multiple_Blocks_Count);
		auto chainConfig = model::BlockChainConfiguration::Uninitialized();
		chainConfig.MaxDifficultyBlocks = Difficulty_History_Size;
		auto cache = test::CreateEmptyCatapultCache(chainConfig);

		// Act: load all difficulties
		context.cacheStorage().loadAll(cache, Height(15));

		// Assert:
		auto view = cache.createView();
		const auto& difficultyView = view.sub<cache::BlockDifficultyCache>();
		ASSERT_EQ(Multiple_Blocks_Count, difficultyView.size());
		auto start = context.elements().cbegin();
		auto end = context.elements().cend();
		AssertDifficultyInfos(start, end, difficultyView);
	}

	TEST(TEST_CLASS, LoadAllDoesNotLoadAnyDifficultiesIfThereAreNoBlocks) {
		// Arrange:
		TestContext context(0);
		auto chainConfig = model::BlockChainConfiguration::Uninitialized();
		chainConfig.MaxDifficultyBlocks = Difficulty_History_Size;
		auto cache = test::CreateEmptyCatapultCache(chainConfig);

		// Act:
		context.cacheStorage().loadAll(cache, Height(15));

		// Assert:
		auto view = cache.createView();
		const auto& difficultyView = view.sub<cache::BlockDifficultyCache>();
		EXPECT_EQ(0u, difficultyView.size());
	}

	// endregion
}}}
