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

#include "harvesting/src/BlockExecutionHashesCalculator.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/extensions/ExecutionConfigurationFactory.h"
#include "catapult/model/Address.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/ResolverTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/other/MockExecutionConfiguration.h"
#include "tests/TestHarness.h"

namespace catapult { namespace harvesting {

#define TEST_CLASS BlockExecutionHashesCalculatorTests

	namespace {
		// region utils

		constexpr auto Currency_Mosaic_Id = MosaicId(1234);
		constexpr auto Harvesting_Mosaic_Id = MosaicId(9876);

		auto CreateConfiguration(bool enableVerifiableState = true) {
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.Network.Identifier = model::NetworkIdentifier::Mijin_Test;
			config.ShouldEnableVerifiableState = enableVerifiableState;
			config.CurrencyMosaicId = Currency_Mosaic_Id;
			config.HarvestingMosaicId = Harvesting_Mosaic_Id;
			config.ImportanceGrouping = 123;
			config.MaxTransactionLifetime = utils::TimeSpan::FromHours(24);
			config.MinHarvesterBalance = Amount(1000);
			config.BlockPruneInterval = 10;
			return config;
		}

		void ZeroTransactionFees(model::Block& block) {
			block.FeeMultiplier = BlockFeeMultiplier(0);
		}

		// endregion

		// region CalculatorTestContext

		// by default use a "real" execution config to test against expected hashes

		struct CalculatorTestContext {
		public:
			CalculatorTestContext() : CalculatorTestContext(CreateConfiguration())
			{}

			explicit CalculatorTestContext(const model::BlockChainConfiguration& config)
					: CalculatorTestContext(config, CreatePluginManagerWithMockTransactionSupport(config))
			{}

			explicit CalculatorTestContext(const std::shared_ptr<plugins::PluginManager>& pPluginManager)
					: CalculatorTestContext(CreateConfiguration(), pPluginManager)
			{}

			explicit CalculatorTestContext(const chain::ExecutionConfiguration& executionConfig)
					: CalculatorTestContext(CreateConfiguration(), nullptr, executionConfig)
			{}

		private:
			CalculatorTestContext(
					const model::BlockChainConfiguration& config,
					const std::shared_ptr<plugins::PluginManager>& pPluginManager)
					: CalculatorTestContext(config, pPluginManager, extensions::CreateExecutionConfiguration(*pPluginManager))
			{}

			CalculatorTestContext(
					const model::BlockChainConfiguration& config,
					const std::shared_ptr<plugins::PluginManager>& pPluginManager,
					const chain::ExecutionConfiguration& executionConfig)
					: m_pPluginManager(pPluginManager)
					, m_config(config)
					, m_executionConfig(executionConfig)
					, m_cache(test::CreateEmptyCatapultCache(m_config, CreateCacheConfiguration(m_dbDirGuard.name()))) {
				test::AddMarkerAccount(m_cache);
				setCacheHeight(Height(1)); // set cache height to nemesis height
			}

		public:
			cache::CatapultCache& cache() {
				return m_cache;
			}

		public:
			void prepareSignerAccount(const Key& signer) {
				prepareSignerAccount(signer, 1, model::ImportanceHeight(1));
			}

			void prepareSignerAccount(const Key& signer, uint32_t multiplier, model::ImportanceHeight importanceHeight) {
				// add the block signer to the cache to avoid state hash changes when there are no transactions
				auto cacheDelta = m_cache.createDelta();

				auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();
				accountStateCacheDelta.addAccount(signer, Height(1));
				auto accountStateIter = accountStateCacheDelta.find(signer);
				accountStateIter.get().Balances.credit(Currency_Mosaic_Id, Amount(1 * multiplier));
				accountStateIter.get().Balances.credit(Harvesting_Mosaic_Id, Amount(1000 * multiplier));
				accountStateIter.get().ImportanceInfo.set(Importance(100), importanceHeight);

				// recalculate the state hash and commit changes
				cacheDelta.calculateStateHash(Height(1));
				m_cache.commit(Height(1));
			}

			void setCacheHeight(Height height) {
				auto delta = m_cache.createDelta();
				m_cache.commit(height);
			}

		public:
			BlockExecutionHashes calculate(const model::Block& block, const std::vector<Hash256>& transactionHashes) const {
				return CalculateBlockExecutionHashes(block, transactionHashes, m_cache, m_config, m_executionConfig);
			}

		private:
			static std::shared_ptr<plugins::PluginManager> CreatePluginManagerWithMockTransactionSupport(
					const model::BlockChainConfiguration& config) {
				auto pPluginManager = test::CreatePluginManager(config);
				pPluginManager->addTransactionSupport(mocks::CreateMockTransactionPlugin());
				return pPluginManager;
			}

			static cache::CacheConfiguration CreateCacheConfiguration(const std::string& databaseDirectory) {
				return cache::CacheConfiguration(databaseDirectory, utils::FileSize(), cache::PatriciaTreeStorageMode::Enabled);
			}

		private:
			test::TempDirectoryGuard m_dbDirGuard;
			std::shared_ptr<plugins::PluginManager> m_pPluginManager;
			model::BlockChainConfiguration m_config;
			chain::ExecutionConfiguration m_executionConfig;
			cache::CatapultCache m_cache;
		};

		// endregion

		// region RunEnabledTest

		enum class StateVerifyOptions { None = 0, State = 1, Receipts = 2, All = 3 };

		constexpr bool HasFlag(StateVerifyOptions testedFlag, StateVerifyOptions value) {
			return utils::to_underlying_type(testedFlag) == (utils::to_underlying_type(testedFlag) & utils::to_underlying_type(value));
		}

		Hash256 CalculateExpectedReceiptsHash(const model::Block& block) {
			model::BlockStatementBuilder blockStatementBuilder;
			auto receiptType = model::Receipt_Type_Harvest_Fee;
			blockStatementBuilder.addReceipt(model::BalanceChangeReceipt(receiptType, block.Signer, Currency_Mosaic_Id, Amount(0)));
			return model::CalculateMerkleHash(*blockStatementBuilder.build());
		}

		template<typename TAssertHashes>
		void RunEnabledTest(StateVerifyOptions verifyOptions, Height blockHeight, uint32_t numTransactions, TAssertHashes assertHashes) {
			// Arrange:
			auto transactionHashes = test::GenerateRandomDataVector<Hash256>(numTransactions);
			auto pBlock = test::GenerateBlockWithTransactions(numTransactions, blockHeight, Timestamp(100));
			auto calculatedReceiptsHash = CalculateExpectedReceiptsHash(*pBlock);
			ZeroTransactionFees(*pBlock);

			// - prepare context
			auto config = CreateConfiguration(HasFlag(StateVerifyOptions::State, verifyOptions));
			config.ShouldEnableVerifiableReceipts = HasFlag(StateVerifyOptions::Receipts, verifyOptions);
			CalculatorTestContext context(config);
			context.prepareSignerAccount(pBlock->Signer);

			auto preCacheStateHash = context.cache().createView().calculateStateHash().StateHash;

			// Act:
			auto blockExecutionHashes = context.calculate(*pBlock, transactionHashes);
			auto postCacheStateHash = context.cache().createView().calculateStateHash().StateHash;

			// Assert: cache state hash should not change
			EXPECT_EQ(preCacheStateHash, postCacheStateHash);

			// - check the block execution dependent hashes
			EXPECT_TRUE(blockExecutionHashes.IsExecutionSuccess);
			assertHashes(calculatedReceiptsHash, preCacheStateHash, blockExecutionHashes);
		}

		// endregion
	}

	// region verifiable state setting honored

	TEST(TEST_CLASS, ZeroHashesAreReturnedWhenVerifiableStateAndReceiptsAreDisabled) {
		// Act:
		RunEnabledTest(StateVerifyOptions::None, Height(2), 3, [](const auto&, const auto&, const auto& blockExecutionHashes) {
			// Assert:
			EXPECT_EQ(Hash256(), blockExecutionHashes.ReceiptsHash);
			EXPECT_EQ(Hash256(), blockExecutionHashes.StateHash);
		});
	}

	TEST(TEST_CLASS, NonZeroStateHashIsReturnedWhenVerifiableStateIsEnabled_NoTransactions) {
		// Act:
		RunEnabledTest(StateVerifyOptions::State, Height(2), 0, [](
				const auto&,
				const auto& cacheStateHash,
				const auto& blockExecutionHashes) {
			// Assert: block does not trigger any account state changes, so hashes should be the same
			EXPECT_EQ(Hash256(), blockExecutionHashes.ReceiptsHash);
			EXPECT_NE(Hash256(), blockExecutionHashes.StateHash);

			EXPECT_EQ(cacheStateHash, blockExecutionHashes.StateHash);
		});
	}

	TEST(TEST_CLASS, NonZeroStateHashIsReturnedWhenVerifiableStateIsEnabled_WithTransactions) {
		// Act:
		RunEnabledTest(StateVerifyOptions::State, Height(2), 3, [](
				const auto&,
				const auto& cacheStateHash,
				const auto& blockExecutionHashes) {
			// Assert: transactions trigger account state changes, so hashes should not be the same
			EXPECT_EQ(Hash256(), blockExecutionHashes.ReceiptsHash);
			EXPECT_NE(Hash256(), blockExecutionHashes.StateHash);

			EXPECT_NE(cacheStateHash, blockExecutionHashes.StateHash);
		});
	}

	TEST(TEST_CLASS, NonZeroReceiptsHashIsReturnedWhenVerifiableReceiptsIsEnabled) {
		// Act:
		RunEnabledTest(StateVerifyOptions::Receipts, Height(2), 0, [](
				const auto& calculatedReceiptsHash,
				const auto&,
				const auto& blockExecutionHashes) {
			// Assert:
			EXPECT_NE(Hash256(), blockExecutionHashes.ReceiptsHash);
			EXPECT_EQ(Hash256(), blockExecutionHashes.StateHash);

			EXPECT_EQ(calculatedReceiptsHash, blockExecutionHashes.ReceiptsHash);
		});
	}

	TEST(TEST_CLASS, NonZeroHashesAreReturnedWhenVerifiableReceiptsAndStateAreEnabled) {
		// Act:
		RunEnabledTest(StateVerifyOptions::All, Height(2), 3, [](
				const auto& calculatedReceiptsHash,
				const auto& cacheStateHash,
				const auto& blockExecutionHashes) {
			// Assert: transactions trigger account state changes, so hashes should not be the same
			EXPECT_NE(Hash256(), blockExecutionHashes.ReceiptsHash);
			EXPECT_NE(Hash256(), blockExecutionHashes.StateHash);

			EXPECT_EQ(calculatedReceiptsHash, blockExecutionHashes.ReceiptsHash);
			EXPECT_NE(cacheStateHash, blockExecutionHashes.StateHash);
		});
	}

	// endregion

	// region state hash dependent on contents

	TEST(TEST_CLASS, DifferentBlocksYieldSameStateHashes) {
		// Arrange: create two blocks with different signers
		auto transactionHashes = test::GenerateRandomDataVector<Hash256>(0);
		auto pBlock1 = test::GenerateBlockWithTransactions(0, Height(2), Timestamp());
		auto pBlock2 = test::CopyBlock(*pBlock1);
		test::FillWithRandomData(pBlock2->Signer);

		// - prepare context
		CalculatorTestContext context;
		context.prepareSignerAccount(pBlock1->Signer);
		context.prepareSignerAccount(pBlock2->Signer);

		// Act:
		auto blockExecutionHashes1 = context.calculate(*pBlock1, transactionHashes);
		auto blockExecutionHashes2 = context.calculate(*pBlock2, transactionHashes);

		// Assert:
		EXPECT_TRUE(blockExecutionHashes1.IsExecutionSuccess);
		EXPECT_TRUE(blockExecutionHashes2.IsExecutionSuccess);

		// - when there are no transactions, blocks will not change state hashes
		EXPECT_EQ(blockExecutionHashes1.StateHash, blockExecutionHashes2.StateHash);
	}

	TEST(TEST_CLASS, DifferentTransactionsYieldDifferentStateHashes) {
		// Arrange: create two blocks with one different transaction signer
		auto transactionHashes = test::GenerateRandomDataVector<Hash256>(3);
		auto pBlock1 = test::GenerateBlockWithTransactions(3, Height(2), Timestamp());
		ZeroTransactionFees(*pBlock1);

		auto pBlock2 = test::CopyBlock(*pBlock1);
		test::FillWithRandomData(pBlock2->Transactions().begin()->Signer);

		// - prepare context
		CalculatorTestContext context;
		context.prepareSignerAccount(pBlock1->Signer); // block signers are the same

		// Act:
		auto blockExecutionHashes1 = context.calculate(*pBlock1, transactionHashes);
		auto blockExecutionHashes2 = context.calculate(*pBlock2, transactionHashes);

		// Assert:
		EXPECT_TRUE(blockExecutionHashes1.IsExecutionSuccess);
		EXPECT_TRUE(blockExecutionHashes2.IsExecutionSuccess);

		// - when there are transactions, blocks will change state hashes (e.g. adding accounts to AccountStateCache)
		EXPECT_NE(blockExecutionHashes1.StateHash, blockExecutionHashes2.StateHash);
	}

	// endregion

	// region state hash importance influence

	TEST(TEST_CLASS, ImportanceIsCalculatedAtProperHeight) {
		// Arrange:
		auto transactionHashes = test::GenerateRandomDataVector<Hash256>(3);
		auto pBlock = test::GenerateBlockWithTransactions(3, Height(2), Timestamp(100));
		ZeroTransactionFees(*pBlock);

		// - prepare context
		CalculatorTestContext context;
		context.prepareSignerAccount(pBlock->Signer);

		// - seed transaction accounts so that importance active harvesting mosaic summation is nonzero
		// - seed recipients and block signer too so all accounts have a constant cache height across all state hash calculations
		{
			auto& cache = context.cache();
			auto cacheDelta = cache.createDelta();
			auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();
			for (const auto& transaction : pBlock->Transactions()) {
				accountStateCacheDelta.addAccount(transaction.Signer, Height(1));
				accountStateCacheDelta.find(transaction.Signer).get().Balances.credit(Harvesting_Mosaic_Id, Amount(1'000'000));

				accountStateCacheDelta.addAccount(static_cast<const mocks::MockTransaction&>(transaction).Recipient, Height(1));
			}

			accountStateCacheDelta.addAccount(pBlock->Signer, Height(1));

			cache.commit(Height(1));
		}

		// Act: height 123 => recalc should occur
		context.setCacheHeight(Height(122));
		pBlock->Height = Height(123);
		auto blockExecutionHashes1 = context.calculate(*pBlock, transactionHashes);

		// - height 124 => signer must have most recent importance to be eligible
		context.prepareSignerAccount(pBlock->Signer, 0, model::ImportanceHeight(123));
		context.setCacheHeight(Height(123));
		pBlock->Height = Height(124);
		auto blockExecutionHashes2 = context.calculate(*pBlock, transactionHashes);

		// - height => 125 no recalc
		context.setCacheHeight(Height(124));
		pBlock->Height = Height(125);
		auto blockExecutionHashes3 = context.calculate(*pBlock, transactionHashes);

		// Assert:
		EXPECT_TRUE(blockExecutionHashes1.IsExecutionSuccess);
		EXPECT_TRUE(blockExecutionHashes2.IsExecutionSuccess);
		EXPECT_TRUE(blockExecutionHashes3.IsExecutionSuccess);

		// - state calculation happened at cache height 123 (importance grouping)
		EXPECT_NE(blockExecutionHashes1.StateHash, blockExecutionHashes2.StateHash);
		EXPECT_EQ(blockExecutionHashes2.StateHash, blockExecutionHashes3.StateHash);
	}

	// endregion

	// region transaction hashes

	namespace {
		DECLARE_OBSERVER(TransactionHashCapture, model::TransactionNotification)(std::vector<Hash256>& capturedHashes) {
			return MAKE_OBSERVER(TransactionHashCapture, model::TransactionNotification, [&capturedHashes](
					const auto& notification,
					const auto&) {
				capturedHashes.push_back(notification.TransactionHash);
			});
		}
	}

	TEST(TEST_CLASS, SuppliedTransactionHashesAreUsed) {
		// Arrange:
		auto transactionHashes = test::GenerateRandomDataVector<Hash256>(7);
		auto pBlock = test::GenerateBlockWithTransactions(7, Height(2), Timestamp(100));
		ZeroTransactionFees(*pBlock);

		// - prepare context
		std::vector<Hash256> capturedHashes;
		auto pPluginManager = test::CreatePluginManager(CreateConfiguration());
		pPluginManager->addTransactionSupport(mocks::CreateMockTransactionPlugin());
		pPluginManager->addObserverHook([&capturedHashes](auto& builder) {
			builder.add(CreateTransactionHashCaptureObserver(capturedHashes));
		});
		CalculatorTestContext context(pPluginManager);
		context.prepareSignerAccount(pBlock->Signer);

		// Act: calculate state hash
		auto blockExecutionHashes = context.calculate(*pBlock, transactionHashes);

		// Assert: transaction hashes are applied correctly
		EXPECT_TRUE(blockExecutionHashes.IsExecutionSuccess);
		EXPECT_EQ(transactionHashes, capturedHashes);
	}

	namespace {
		void AssertMismatchedTransactionHashes(size_t numBlockTransactions, size_t numTransactionHashes) {
			// Arrange:
			auto transactionHashes = test::GenerateRandomDataVector<Hash256>(numTransactionHashes);
			auto pBlock = test::GenerateBlockWithTransactionsAtHeight(numBlockTransactions, Height(2));
			ZeroTransactionFees(*pBlock);

			// - prepare context
			CalculatorTestContext context;
			context.prepareSignerAccount(pBlock->Signer);

			// Act + Assert:
			EXPECT_THROW(context.calculate(*pBlock, transactionHashes), catapult_invalid_argument);
		}
	}

	TEST(TEST_CLASS, CalculationFailsWhenTooFewTransactionHashesAreProvided) {
		// Assert:
		AssertMismatchedTransactionHashes(7, 0);
		AssertMismatchedTransactionHashes(7, 4);
		AssertMismatchedTransactionHashes(7, 6);
	}

	TEST(TEST_CLASS, CalculationFailsWhenTooManyTransactionHashesAreProvided) {
		// Assert:
		AssertMismatchedTransactionHashes(7, 8);
		AssertMismatchedTransactionHashes(7, 10);
		AssertMismatchedTransactionHashes(7, 14);
	}

	// endregion

	// region state hash uses resolvers

	namespace {
		DECLARE_OBSERVER(ResolvedMosaicCapture, model::BalanceTransferNotification)(std::vector<MosaicId>& capturedResolvedMosaics) {
			return MAKE_OBSERVER(ResolvedMosaicCapture, model::BalanceTransferNotification, [&capturedResolvedMosaics](
					const auto& notification,
					const auto& context) {
				capturedResolvedMosaics.push_back(context.Resolvers.resolve(notification.MosaicId));
			});
		}

		void PreparePluginManager(plugins::PluginManager& pluginManager, std::vector<MosaicId>& capturedResolvedMosaics) {
			// 1. enable Publish_Transfers (MockTransaction Publish XORs recipient address, so XOR address resolver is required
			//    for proper roundtripping or else test will fail)
			pluginManager.addTransactionSupport(mocks::CreateMockTransactionPlugin(mocks::PluginOptionFlags::Publish_Transfers));

			// 2. create custom XOR mosaic resolver that is dependent on cache (size)
			pluginManager.addMosaicResolver([](const auto& readOnlyCache, const auto& unresolved, auto& resolved) {
				auto numAccounts = readOnlyCache.template sub<cache::AccountStateCache>().size();
				resolved = test::CreateResolverContextXor().resolve(unresolved + UnresolvedMosaicId(numAccounts));
				return true;
			});

			// 3. configure default XOR address resolver
			pluginManager.addAddressResolver([](const auto&, const auto& unresolved, auto& resolved) {
				resolved = test::CreateResolverContextXor().resolve(unresolved);
				return true;
			});

			// 4. add observer that captures resolved mosaics
			pluginManager.addObserverHook([&capturedResolvedMosaics](auto& builder) {
				builder.add(CreateResolvedMosaicCaptureObserver(capturedResolvedMosaics));
			});
		}
	}

	TEST(TEST_CLASS, StateHashCalculationCreatesAppropriateResolvers) {
		// Arrange: prepare context and configure custom resolvers for this test
		auto blockSigner = test::GenerateRandomData<Key_Size>();
		std::vector<MosaicId> resolvedMosaics;
		auto pPluginManager = test::CreatePluginManager(CreateConfiguration());
		PreparePluginManager(*pPluginManager, resolvedMosaics);
		CalculatorTestContext context(pPluginManager);
		context.prepareSignerAccount(blockSigner);

		// - create a block with two mock transactions and initialize the account state cache
		constexpr auto Num_Accounts = 6u; // 2 * (2 per transaction) + block + marker
		std::vector<model::UnresolvedMosaic> mosaics{ { UnresolvedMosaicId(123), Amount(100) }, { UnresolvedMosaicId(256), Amount(200) } };
		test::ConstTransactions transactions;
		{
			auto& cache = context.cache();
			auto cacheDelta = cache.createDelta();
			auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();
			for (const auto& mosaic : mosaics) {
				// 1. create a transaction with the specified mosaic
				auto pTransaction = mocks::CreateTransactionWithFeeAndTransfers(Amount(), { mosaic });
				pTransaction->Deadline = Timestamp(101);

				// 2. calculate the expected resolved mosaic - it should be offset by number of accounts and XORed
				auto resolvedMosaicId = test::CreateResolverContextXor().resolve(mosaic.MosaicId + UnresolvedMosaicId(Num_Accounts));

				// 3. credit the signer
				accountStateCacheDelta.addAccount(pTransaction->Signer, Height(1));
				accountStateCacheDelta.find(pTransaction->Signer).get().Balances.credit(resolvedMosaicId, mosaic.Amount);

				// 4. add the recipient to the cache
				auto recipient = PublicKeyToAddress(pTransaction->Recipient, model::NetworkIdentifier::Mijin_Test);
				accountStateCacheDelta.addAccount(recipient, Height(1));

				transactions.push_back(std::move(pTransaction));
			}

			cache.commit(Height(1));
		}

		auto transactionHashes = test::GenerateRandomDataVector<Hash256>(transactions.size());
		auto pBlock = test::GenerateRandomBlockWithTransactions(transactions);
		pBlock->Signer = blockSigner;
		pBlock->Timestamp = Timestamp(100);
		ZeroTransactionFees(*pBlock);
		pBlock->Height = Height(2);

		// Act: calculate state hash
		auto blockExecutionHashes = context.calculate(*pBlock, transactionHashes);

		// Assert: mosaics were resolved correctly (and offset by number of accounts in cache)
		EXPECT_TRUE(blockExecutionHashes.IsExecutionSuccess);

		ASSERT_EQ(2u, resolvedMosaics.size());
		EXPECT_EQ(MosaicId(test::UnresolveXor(MosaicId(123 + Num_Accounts)).unwrap()), resolvedMosaics[0]);
		EXPECT_EQ(MosaicId(test::UnresolveXor(MosaicId(256 + Num_Accounts)).unwrap()), resolvedMosaics[1]);
	}

	// endregion

	// region stateful validation

	namespace {
		template<typename TParams>
		void AssertEntityInfos(
				const TParams& params,
				const std::vector<Hash256>& expectedTransactionHashes,
				size_t numExpected,
				const std::string& tag) {
			// Assert:
			ASSERT_EQ(numExpected, params.size());

			// block hash is last hash AND always zero
			for (auto i = 0u; i < params.size(); ++i) {
				auto expectedHash = (i / 2 >= expectedTransactionHashes.size()) ? Hash256() : expectedTransactionHashes[i / 2];
				EXPECT_EQ(expectedHash, params[i].HashCopy) << tag << " param at " << i;
				EXPECT_EQ(0 == i % 2 ? 1u : 2u, params[i].SequenceId) << tag << " param at " << i;
			}
		}

		void AssertValidatorContexts(
				const test::MockExecutionConfiguration& executionConfig,
				Height expectedHeight,
				Timestamp expectedTimestamp,
				const std::vector<size_t>& expectedNumDifficultyInfos) {
			// Assert:
			test::MockExecutionConfiguration::AssertValidatorContexts(
					*executionConfig.pValidator,
					expectedNumDifficultyInfos,
					expectedHeight,
					expectedTimestamp);
		}

		void AssertObserverContexts(
				const test::MockExecutionConfiguration& executionConfig,
				Height expectedHeight,
				size_t numObserverCalls) {
			// Assert:
			EXPECT_EQ(numObserverCalls, executionConfig.pObserver->params().size());
			test::MockExecutionConfiguration::AssertObserverContexts(
					*executionConfig.pObserver,
					0,
					expectedHeight,
					model::ImportanceHeight(1),
					[](auto) { return false; });
		}
	}

	TEST(TEST_CLASS, CanCalculateHashesWhenValidationSucceedsForAll) {
		// Arrange: prepare context
		test::MockExecutionConfiguration executionConfig;
		CalculatorTestContext context(executionConfig.Config);

		// - prepare a block with transactions
		auto transactionHashes = test::GenerateRandomDataVector<Hash256>(4);
		auto pBlock = test::GenerateBlockWithTransactionsAtHeight(4, Height(2));
		pBlock->Timestamp = Timestamp(100);

		// Act: calculate hashes
		auto blockExecutionHashes = context.calculate(*pBlock, transactionHashes);

		// Assert: validator and observer should be called for all notifications, 2 per entity
		EXPECT_TRUE(blockExecutionHashes.IsExecutionSuccess);

		// - check entity infos (hashes)
		AssertEntityInfos(executionConfig.pValidator->params(), transactionHashes, 10, "validator");
		AssertEntityInfos(executionConfig.pObserver->params(), transactionHashes, 10, "observer");

		// - check contexts
		AssertValidatorContexts(executionConfig, pBlock->Height, pBlock->Timestamp, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 });
		AssertObserverContexts(executionConfig, pBlock->Height, 10);
	}

	namespace {
		void AssertCalculateHashesAbortsOnResult(validators::ValidationResult result) {
			// Arrange: prepare context
			test::MockExecutionConfiguration executionConfig;
			CalculatorTestContext context(executionConfig.Config);

			// - prepare a block with transactions
			auto transactionHashes = test::GenerateRandomDataVector<Hash256>(4);
			auto pBlock = test::GenerateBlockWithTransactionsAtHeight(4, Height(2));
			pBlock->Timestamp = Timestamp(100);

			// - mark a failure on the second transaction
			executionConfig.pValidator->setResult(result, transactionHashes[1], 1);

			// Act: calculate hashes
			auto blockExecutionHashes = context.calculate(*pBlock, transactionHashes);

			// Assert: validator should be called for tx1 (2) and tx2 (1); observer should be called for tx1 (2)
			EXPECT_FALSE(blockExecutionHashes.IsExecutionSuccess);

			// - check entity infos (hashes)
			AssertEntityInfos(executionConfig.pValidator->params(), transactionHashes, 3, "validator");
			AssertEntityInfos(executionConfig.pObserver->params(), transactionHashes, 2, "observer");

			// - check contexts
			AssertValidatorContexts(executionConfig, pBlock->Height, pBlock->Timestamp, { 0, 1, 2 });
			AssertObserverContexts(executionConfig, pBlock->Height, 2);
		}
	}

	TEST(TEST_CLASS, CalculateHashesAbortsOnFirstValidationNeutralResult) {
		// Assert:
		AssertCalculateHashesAbortsOnResult(validators::ValidationResult::Neutral);
	}

	TEST(TEST_CLASS, CalculateHashesAbortsOnFirstValidationFailureResult) {
		// Assert:
		AssertCalculateHashesAbortsOnResult(validators::ValidationResult::Failure);
	}

	// endregion
}}
