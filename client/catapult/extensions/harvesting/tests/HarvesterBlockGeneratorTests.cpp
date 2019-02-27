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

#include "harvesting/src/HarvesterBlockGenerator.h"
#include "harvesting/src/HarvestingUtFacadeFactory.h"
#include "catapult/cache/MemoryUtCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/BlockUtils.h"
#include "tests/test/cache/UtTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/other/MockExecutionConfiguration.h"
#include "tests/TestHarness.h"

namespace catapult { namespace harvesting {

#define TEST_CLASS HarvesterBlockGeneratorTests

	namespace {
		constexpr auto Cache_Height = Height(7);

		// region test context

		auto CreateBlockChainConfiguration() {
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.ShouldEnableVerifiableState = true;
			config.ShouldEnableVerifiableReceipts = true;
			config.CurrencyMosaicId = MosaicId(123);
			config.ImportanceGrouping = 1;
			return config;
		}

		class TestContext {
		public:
			explicit TestContext(model::TransactionSelectionStrategy strategy)
					: m_config(CreateBlockChainConfiguration())
					, m_catapultCache(test::CreateEmptyCatapultCache(m_config, CreateCacheConfiguration(m_dbDirGuard.name())))
					, m_utFacadeFactory(m_catapultCache, m_config, m_executionConfig.Config)
					, m_pUtCache(test::CreateSeededMemoryUtCache(0))
					, m_generator(CreateHarvesterBlockGenerator(strategy, m_utFacadeFactory, *m_pUtCache)) {
				// add 5 transaction infos to UT cache with multipliers alternating between 10 and 20
				auto transactionInfos = test::CreateTransactionInfosFromSizeMultiplierPairs({
					{ 201, 200 }, { 202, 100 }, { 203, 200 }, { 204, 100 }, { 205, 200 }
				});
				test::AddAll(*m_pUtCache, transactionInfos);

				// add accounts to cache for fix up support
				auto cacheDelta = m_catapultCache.createDelta();
				auto& accountStateCache = cacheDelta.sub<cache::AccountStateCache>();
				for (const auto& transactionInfo : transactionInfos)
					accountStateCache.addAccount(transactionInfo.pEntity->Signer, Cache_Height);

				// force state hash recalculation and commit
				m_initialStateHash = cacheDelta.calculateStateHash(Cache_Height).StateHash;
				m_catapultCache.commit(Cache_Height);
			}

		public:
			const Hash256& initialStateHash() const {
				return m_initialStateHash;
			}

		public:
			auto generate(Height blockHeight, uint32_t maxTransactionsPerBlock) {
				model::BlockHeader blockHeader;
				blockHeader.Height = blockHeight;
				return m_generator(blockHeader, maxTransactionsPerBlock);
			}

		public:
			void setValidationFailure() {
				m_executionConfig.pValidator->setResult(validators::ValidationResult::Failure);
			}

		private:
			static cache::CacheConfiguration CreateCacheConfiguration(const std::string& databaseDirectory) {
				return cache::CacheConfiguration(databaseDirectory, utils::FileSize(), cache::PatriciaTreeStorageMode::Enabled);
			}

		private:
			test::TempDirectoryGuard m_dbDirGuard;
			model::BlockChainConfiguration m_config;
			cache::CatapultCache m_catapultCache;
			test::MockExecutionConfiguration m_executionConfig;
			HarvestingUtFacadeFactory m_utFacadeFactory;
			std::unique_ptr<cache::MemoryUtCache> m_pUtCache;
			BlockGenerator m_generator;

			Hash256 m_initialStateHash;
		};

		// endregion
	}

	// region generation failure

	TEST(TEST_CLASS, GenerationFailsWhenBlockHeightMismatchDetected) {
		// Arrange:
		TestContext context(model::TransactionSelectionStrategy::Oldest);

		// Act: use mismatched height
		auto pBlock = context.generate(Cache_Height, 4);

		// Assert:
		EXPECT_FALSE(!!pBlock);
	}

	TEST(TEST_CLASS, GenerationFailsWhenUtProcessingFails) {
		// Arrange: set validation failure
		TestContext context(model::TransactionSelectionStrategy::Oldest);
		context.setValidationFailure();

		// Act:
		auto pBlock = context.generate(Cache_Height + Height(1), 4);

		// Assert:
		EXPECT_FALSE(!!pBlock);
	}

	// endregion

	// region generation success

	TEST(TEST_CLASS, CanGenerateBlockWithoutTransactions) {
		// Arrange:
		TestContext context(model::TransactionSelectionStrategy::Oldest);

		// Act:
		auto pBlock = context.generate(Cache_Height + Height(1), 0);

		// Assert:
		ASSERT_TRUE(!!pBlock);
		EXPECT_EQ(0u, model::CalculateBlockTransactionsInfo(*pBlock).Count);

		// - zeroed because no transactions
		EXPECT_EQ(Hash256(), pBlock->BlockTransactionsHash);
		EXPECT_EQ(BlockFeeMultiplier(0), pBlock->FeeMultiplier);

		// - no state changes and no receipts generated
		EXPECT_EQ(context.initialStateHash(), pBlock->StateHash);
		EXPECT_EQ(Hash256(), pBlock->BlockReceiptsHash);
	}

	TEST(TEST_CLASS, CanGenerateBlockWithTransactions) {
		// Arrange:
		TestContext context(model::TransactionSelectionStrategy::Oldest);

		// Act:
		auto pBlock = context.generate(Cache_Height + Height(1), 4);

		// Assert: 4 (deterministic) transactions should have been added to the block
		ASSERT_TRUE(!!pBlock);
		EXPECT_EQ(4u, model::CalculateBlockTransactionsInfo(*pBlock).Count);

		auto i = 0u;
		for (const auto& transaction : pBlock->Transactions()) {
			EXPECT_EQ(201u + i, transaction.Size) << "transaction at " << i;
			++i;
		}

		// - nonzero because transactions present
		EXPECT_NE(Hash256(), pBlock->BlockTransactionsHash);
		EXPECT_EQ(BlockFeeMultiplier(10), pBlock->FeeMultiplier);

		// - state changes but no receipts generated
		EXPECT_NE(context.initialStateHash(), pBlock->StateHash);
		EXPECT_EQ(Hash256(), pBlock->BlockReceiptsHash);
	}

	// endregion
}}
