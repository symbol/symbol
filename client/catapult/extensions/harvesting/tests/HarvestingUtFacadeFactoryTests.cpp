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

#include "harvesting/src/HarvestingUtFacadeFactory.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/Address.h"
#include "catapult/model/BlockUtils.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/test/other/MockExecutionConfiguration.h"
#include "tests/TestHarness.h"

namespace catapult { namespace harvesting {

#define TEST_CLASS HarvestingUtFacadeFactoryTests

	namespace {
		// region constants

		constexpr auto Default_Height = Height(17);
		constexpr auto Default_Time = Timestamp(987);
		constexpr auto Default_Last_Recalculation_Height = model::ImportanceHeight(1234);

		constexpr auto Network_Identifier = model::NetworkIdentifier::Private_Test;
		constexpr auto Currency_Mosaic_Id = MosaicId(1234);
		constexpr auto Harvesting_Mosaic_Id = MosaicId(9876);

		// endregion

		// region CreateBlockChainConfiguration

		enum class StateVerifyOptions { None = 0, State = 1, Receipts = 2, All = 3 };

		constexpr bool HasFlag(StateVerifyOptions testedFlag, StateVerifyOptions value) {
			return utils::to_underlying_type(testedFlag) == (utils::to_underlying_type(testedFlag) & utils::to_underlying_type(value));
		}

		auto CreateBlockChainConfiguration(StateVerifyOptions verifyOptions = StateVerifyOptions::None) {
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.Network.Identifier = Network_Identifier;
			config.EnableVerifiableState = HasFlag(StateVerifyOptions::State, verifyOptions);
			config.EnableVerifiableReceipts = HasFlag(StateVerifyOptions::Receipts, verifyOptions);
			config.CurrencyMosaicId = Currency_Mosaic_Id;
			config.HarvestingMosaicId = Harvesting_Mosaic_Id;
			config.ImportanceGrouping = 4;
			config.MaxTransactionLifetime = utils::TimeSpan::FromHours(24);
			config.MinHarvesterBalance = Amount(1000);
			return config;
		}

		// endregion

		// region RunUtFacadeTest / AssertEmpty

		void SetDependentState(cache::CatapultCache& catapultCache) {
			auto delta = catapultCache.createDelta();
			delta.dependentState().LastRecalculationHeight = Default_Last_Recalculation_Height;
			catapultCache.commit(Default_Height);
		}

		template<typename TAction>
		void RunUtFacadeTest(TAction action) {
			// Arrange: create factory and facade
			auto catapultCache = test::CreateCatapultCacheWithMarkerAccount(Default_Height);
			SetDependentState(catapultCache);

			test::MockExecutionConfiguration executionConfig;
			HarvestingUtFacadeFactory factory(catapultCache, CreateBlockChainConfiguration(), executionConfig.Config);

			auto pFacade = factory.create(Default_Time);
			ASSERT_TRUE(!!pFacade);

			// Act + Assert:
			action(*pFacade, executionConfig);
		}

		template<typename TAction>
		void RunUtFacadeTest(uint32_t numTransactionInfos, TAction action) {
			// Arrange: create transactions with zero max fees (this causes none to have a surplus when block fee multiplier is zero)
			std::vector<std::pair<uint32_t, uint32_t>> sizeMultiplierPairs;
			for (auto i = 0u; i < numTransactionInfos; ++i)
				sizeMultiplierPairs.emplace_back(200, 0);

			auto transactionInfos = test::CreateTransactionInfosFromSizeMultiplierPairs(sizeMultiplierPairs);

			// - create factory and facade
			auto catapultCache = test::CreateCatapultCacheWithMarkerAccount(Default_Height);
			SetDependentState(catapultCache);

			test::MockExecutionConfiguration executionConfig;
			HarvestingUtFacadeFactory factory(catapultCache, CreateBlockChainConfiguration(), executionConfig.Config);

			auto pFacade = factory.create(Default_Time);
			ASSERT_TRUE(!!pFacade);

			// Act + Assert:
			action(*pFacade, transactionInfos, executionConfig);
		}

		void AssertEmpty(const HarvestingUtFacade& facade) {
			// Assert:
			EXPECT_EQ(Default_Height + Height(1), facade.height());
			EXPECT_EQ(0u, facade.size());
			EXPECT_EQ(0u, facade.transactionInfos().size());
		}

		// endregion

		// region entity / context asserts

		template<typename TParams>
		void AssertEntityInfos(
				const std::string& tag,
				const TParams& params,
				const std::vector<Hash256>& transactionHashes,
				const std::vector<std::pair<size_t, size_t>>& expectedIndexIdPairs) {
			// Assert:
			ASSERT_EQ(expectedIndexIdPairs.size(), params.size());

			for (auto i = 0u; i < params.size(); ++i) {
				const auto& pair = expectedIndexIdPairs[i];
				EXPECT_EQ(transactionHashes[pair.first], params[i].HashCopy) << tag << " param at " << i;
				EXPECT_EQ(pair.second, params[i].SequenceId) << tag << " param at " << i;
			}
		}

		void AssertValidatorContexts(
				const test::MockExecutionConfiguration& executionConfig,
				const std::vector<size_t>& expectedNumStatistics) {
			// Assert:
			test::MockExecutionConfiguration::AssertValidatorContexts(
					*executionConfig.pValidator,
					expectedNumStatistics,
					Default_Height + Height(1),
					Default_Time);
		}

		void AssertObserverContexts(
				const test::MockExecutionConfiguration& executionConfig,
				size_t numObserverCalls,
				const std::unordered_set<size_t>& rollbackIndexes) {
			// Assert:
			EXPECT_EQ(numObserverCalls, executionConfig.pObserver->params().size());
			test::MockExecutionConfiguration::AssertObserverContexts(
					*executionConfig.pObserver,
					0,
					Default_Height + Height(1),
					Default_Last_Recalculation_Height,
					[&rollbackIndexes](auto i) { return rollbackIndexes.cend() != rollbackIndexes.find(i); });
		}

		void AssertObserverContexts(const test::MockExecutionConfiguration& executionConfig, size_t numObserverCalls) {
			AssertObserverContexts(executionConfig, numObserverCalls, {});
		}

		// endregion
	}

	// region constructor

	TEST(TEST_CLASS, FacadeIsInitiallyEmpty) {
		// Act:
		RunUtFacadeTest([](const auto& facade, const auto&) {
			// Assert:
			AssertEmpty(facade);
		});
	}

	// endregion

	// region apply

	TEST(TEST_CLASS, CanApplyTransactionsToCache_AllSuccess) {
		// Arrange:
		RunUtFacadeTest([](auto& facade, const auto& executionConfig) {
			auto transactionInfos = test::CreateTransactionInfos(4);
			auto transactionHashes = test::ExtractHashes(transactionInfos);

			// Act: apply transactions to the cache
			std::vector<bool> applyResults;
			for (const auto& transactionInfo : transactionInfos)
				applyResults.push_back(facade.apply(transactionInfo));

			// Assert:
			EXPECT_EQ(std::vector<bool>(4, true), applyResults);

			// - all transactions were added to ut cache
			EXPECT_EQ(Default_Height + Height(1), facade.height());
			EXPECT_EQ(4u, facade.size());
			test::AssertEquivalent(transactionInfos, facade.transactionInfos());

			// - check entity infos (hashes): validator and observer should be called for all notifications (2 per transaction)
			std::vector<std::pair<size_t, size_t>> expectedIndexIdPairs{
				{ 0, 1 }, { 0, 2 }, { 1, 1 }, { 1, 2 }, { 2, 1 }, { 2, 2 }, { 3, 1 }, { 3, 2 }
			};
			AssertEntityInfos("validator", executionConfig.pValidator->params(), transactionHashes, expectedIndexIdPairs);
			AssertEntityInfos("observer", executionConfig.pObserver->params(), transactionHashes, expectedIndexIdPairs);

			// - check contexts
			AssertValidatorContexts(executionConfig, { 0, 1, 2, 3, 4, 5, 6, 7 });
			AssertObserverContexts(executionConfig, 8);
		});
	}

	TEST(TEST_CLASS, CanApplyTransactionsToCache_AllFailure) {
		// Arrange:
		RunUtFacadeTest([](auto& facade, const auto& executionConfig) {
			auto transactionInfos = test::CreateTransactionInfos(4);
			auto transactionHashes = test::ExtractHashes(transactionInfos);

			// - mark all failures
			executionConfig.pValidator->setResult(validators::ValidationResult::Failure);

			// Act: apply transactions to the cache
			std::vector<bool> applyResults;
			for (const auto& transactionInfo : transactionInfos)
				applyResults.push_back(facade.apply(transactionInfo));

			// Assert:
			EXPECT_EQ(std::vector<bool>(4, false), applyResults);

			// - no transactions were added to ut cache
			AssertEmpty(facade);

			// - check entity infos (hashes): validator should be called for first notifications; observer should never be called
			AssertEntityInfos("validator", executionConfig.pValidator->params(), transactionHashes, {
				{ 0, 1 }, { 1, 1 }, { 2, 1 }, { 3, 1 }
			});
			AssertEntityInfos("observer", executionConfig.pObserver->params(), transactionHashes, {});

			// - check contexts
			AssertValidatorContexts(executionConfig, { 0, 0, 0, 0 });
			AssertObserverContexts(executionConfig, 0);
		});
	}

	namespace {
		template<typename TAssertEntityInfosAndContexts>
		void AssertCanApplyTransactionsSomeSuccess(size_t idTrigger, TAssertEntityInfosAndContexts assertEntityInfosAndContexts) {
			// Arrange:
			RunUtFacadeTest([idTrigger, assertEntityInfosAndContexts](auto& facade, const auto& executionConfig) {
				auto transactionInfos = test::CreateTransactionInfos(4);
				auto transactionHashes = test::ExtractHashes(transactionInfos);

				// - mark some failures
				executionConfig.pValidator->setResult(validators::ValidationResult::Failure, transactionHashes[1], idTrigger);
				executionConfig.pValidator->setResult(validators::ValidationResult::Failure, transactionHashes[3], idTrigger);

				// Act: apply transactions to the cache
				std::vector<bool> applyResults;
				for (const auto& transactionInfo : transactionInfos)
					applyResults.push_back(facade.apply(transactionInfo));

				// Assert:
				EXPECT_EQ(std::vector<bool>({ true, false, true, false }), applyResults);

				// - only two transactions were added to ut cache
				EXPECT_EQ(Default_Height + Height(1), facade.height());
				EXPECT_EQ(2u, facade.size());

				std::vector<model::TransactionInfo> expectedTransactionInfos;
				expectedTransactionInfos.push_back(transactionInfos[0].copy());
				expectedTransactionInfos.push_back(transactionInfos[2].copy());
				test::AssertEquivalent(expectedTransactionInfos, facade.transactionInfos());

				// - check entity infos and contexts
				assertEntityInfosAndContexts(transactionHashes, executionConfig);
			});
		}
	}

	TEST(TEST_CLASS, CanApplyTransactionsToCache_SomeSuccess) {
		// Act:
		AssertCanApplyTransactionsSomeSuccess(1, [](const auto& transactionHashes, const auto& executionConfig) {
			// Assert: check entity infos (hashes): since first notifications failed, no undos are necessary
			AssertEntityInfos("validator", executionConfig.pValidator->params(), transactionHashes, {
				{ 0, 1 }, { 0, 2 }, { 1, 1 }, { 2, 1 }, { 2, 2 }, { 3, 1 }
			});
			AssertEntityInfos("observer", executionConfig.pObserver->params(), transactionHashes, {
				{ 0, 1 }, { 0, 2 }, { 2, 1 }, { 2, 2 }
			});

			AssertValidatorContexts(executionConfig, { 0, 1, 2, 2, 3, 4 });
			AssertObserverContexts(executionConfig, 4);
		});
	}

	TEST(TEST_CLASS, CanApplyTransactionsToCache_SomeSuccessSomeUndos) {
		// Act:
		AssertCanApplyTransactionsSomeSuccess(2, [](const auto& transactionHashes, const auto& executionConfig) {
			// Assert: check entity infos (hashes): since second notifications failed, undos are necessary
			AssertEntityInfos("validator", executionConfig.pValidator->params(), transactionHashes, {
				{ 0, 1 }, { 0, 2 }, { 1, 1 }, { 1, 2 }, { 2, 1 }, { 2, 2 }, { 3, 1 }, { 3, 2 }
			});
			AssertEntityInfos("observer", executionConfig.pObserver->params(), transactionHashes, {
				{ 0, 1 }, { 0, 2 }, { 1, 1 }, { 1, 1 }, { 2, 1 }, { 2, 2 }, { 3, 1 }, { 3, 1 }
			});

			AssertValidatorContexts(executionConfig, { 0, 1, 2, 3, 4, 5, 6, 7 });
			AssertObserverContexts(executionConfig, 8, { 3, 7 });
		});
	}

	// endregion

	// region unapply

	// only one test is required because unapply does not perform notification validation

	TEST(TEST_CLASS, CanUnapplyTransactionsFromCache) {
		// Arrange:
		RunUtFacadeTest([](auto& facade, const auto& executionConfig) {
			auto transactionInfos = test::CreateTransactionInfos(4);
			auto transactionHashes = test::ExtractHashes(transactionInfos);

			// - seed facade with four transactions
			for (const auto& transactionInfo : transactionInfos)
				facade.apply(transactionInfo);

			// Act: unapply (last) three transactions
			for (auto i = 0u; i < 3; ++i)
				facade.unapply();

			// Assert: only first transaction remains in ut cache
			EXPECT_EQ(Default_Height + Height(1), facade.height());
			EXPECT_EQ(1u, facade.size());

			std::vector<model::TransactionInfo> expectedTransactionInfos;
			expectedTransactionInfos.push_back(transactionInfos[0].copy());
			test::AssertEquivalent(expectedTransactionInfos, facade.transactionInfos());

			// - check entity infos (hashes): validator should be called for all notifications (2 per transaction),
			//   observer should be called additionally for last three transactions
			AssertEntityInfos("validator", executionConfig.pValidator->params(), transactionHashes, {
				{ 0, 1 }, { 0, 2 }, { 1, 1 }, { 1, 2 }, { 2, 1 }, { 2, 2 }, { 3, 1 }, { 3, 2 }
			});
			AssertEntityInfos("observer", executionConfig.pObserver->params(), transactionHashes, {
				{ 0, 1 }, { 0, 2 }, { 1, 1 }, { 1, 2 }, { 2, 1 }, { 2, 2 }, { 3, 1 }, { 3, 2 },
				{ 3, 2 }, { 3, 1 }, { 2, 2 }, { 2, 1 }, { 1, 2 }, { 1, 1 }
			});

			// - check contexts
			AssertValidatorContexts(executionConfig, { 0, 1, 2, 3, 4, 5, 6, 7 });
			AssertObserverContexts(executionConfig, 14, { 8, 9, 10, 11, 12, 13 });
		});
	}

	TEST(TEST_CLASS, CannotUnapplyTransactionsWhenEmpty) {
		// Arrange:
		RunUtFacadeTest([](auto& facade, const auto&) {
			auto transactionInfos = test::CreateTransactionInfos(4);

			// - seed facade with four transactions
			for (const auto& transactionInfo : transactionInfos)
				facade.apply(transactionInfo);

			// - unapply all transactions
			for (auto i = 0u; i < transactionInfos.size(); ++i)
				facade.unapply();

			// Act + Assert:
			EXPECT_THROW(facade.unapply(), catapult_out_of_range);
		});
	}

	// endregion

	// region commit - validation

	namespace {
		std::unique_ptr<model::BlockHeader> CreateBlockHeaderWithHeight(Height height) {
			auto pBlockHeader = std::make_unique<model::BlockHeader>();
			test::FillWithRandomData({ reinterpret_cast<uint8_t*>(pBlockHeader.get()), sizeof(model::BlockHeader) });
			pBlockHeader->Size = sizeof(model::BlockHeader);
			pBlockHeader->Height = height;
			pBlockHeader->FeeMultiplier = BlockFeeMultiplier();
			pBlockHeader->ReceiptsHash = Hash256();
			pBlockHeader->StateHash = Hash256();
			return pBlockHeader;
		}
	}

	TEST(TEST_CLASS, CommitFailsWhenBlockHeaderIsInconsistentWithCacheState) {
		// Arrange:
		RunUtFacadeTest(4, [](auto& facade, const auto& transactionInfos, const auto&) {
			// - seed facade with four transactions
			for (const auto& transactionInfo : transactionInfos)
				facade.apply(transactionInfo);

			// Act + Assert: commit
			auto pBlockHeader = CreateBlockHeaderWithHeight(Default_Height);
			EXPECT_THROW(facade.commit(*pBlockHeader), catapult_runtime_error);
		});
	}

	TEST(TEST_CLASS, CommitFailsWhenBlockHeaderFailsValidation) {
		// Arrange:
		RunUtFacadeTest(4, [](auto& facade, const auto& transactionInfos, const auto& executionConfig) {
			// - seed facade with four transactions
			for (const auto& transactionInfo : transactionInfos)
				facade.apply(transactionInfo);

			auto transactionHashes = test::ExtractHashes(transactionInfos);
			transactionHashes.push_back(Hash256()); // zero hash is used for block

			// - fail block validation
			executionConfig.pValidator->setResult(validators::ValidationResult::Failure);

			// Act: commit
			auto pBlockHeader = CreateBlockHeaderWithHeight(Default_Height + Height(1));
			auto pBlock = facade.commit(*pBlockHeader);

			// Assert: transactions have been removed
			AssertEmpty(facade);

			// - check entity infos (hashes): validator fails on first block part
			AssertEntityInfos("validator", executionConfig.pValidator->params(), transactionHashes, {
				{ 0, 1 }, { 0, 2 }, { 1, 1 }, { 1, 2 }, { 2, 1 }, { 2, 2 }, { 3, 1 }, { 3, 2 },
				{ 4, 1 }
			});
			AssertEntityInfos("observer", executionConfig.pObserver->params(), transactionHashes, {
				{ 0, 1 }, { 0, 2 }, { 1, 1 }, { 1, 2 }, { 2, 1 }, { 2, 2 }, { 3, 1 }, { 3, 2 }
			});

			// - check contexts
			AssertValidatorContexts(executionConfig, { 0, 1, 2, 3, 4, 5, 6, 7, 8 });
			AssertObserverContexts(executionConfig, 8);

			// - no block was generated
			EXPECT_FALSE(!!pBlock);
		});
	}

	TEST(TEST_CLASS, CommitProducesBlockWhenValidationPasses_WithoutTransactions) {
		// Arrange:
		RunUtFacadeTest(0, [](auto& facade, const auto&, const auto& executionConfig) {
			// Act: commit
			auto pBlockHeader = CreateBlockHeaderWithHeight(Default_Height + Height(1));
			auto pBlock = facade.commit(*pBlockHeader);

			// Assert: transactions have been removed
			AssertEmpty(facade);

			// - check entity infos (hashes): validator and observer should be called for all notifications (2 per transaction and block)
			AssertEntityInfos("validator", executionConfig.pValidator->params(), { Hash256() }, { { 0, 1 }, { 0, 2 } });
			AssertEntityInfos("observer", executionConfig.pObserver->params(), { Hash256() }, { { 0, 1 }, { 0, 2 } });

			// - check contexts
			AssertValidatorContexts(executionConfig, { 0, 1 });
			AssertObserverContexts(executionConfig, 2);

			// - check block
			EXPECT_EQ_MEMORY(pBlockHeader.get(), pBlock.get(), sizeof(model::BlockHeader));
			EXPECT_EQ(0u, model::CalculateBlockTransactionsInfo(*pBlock).Count);
		});
	}

	TEST(TEST_CLASS, CommitProducesBlockWhenValidationPasses_WithTransactions) {
		// Arrange:
		RunUtFacadeTest(4, [](auto& facade, const auto& transactionInfos, const auto& executionConfig) {
			// - seed facade with four transactions
			auto transactionsSize = 0u;
			for (const auto& transactionInfo : transactionInfos) {
				facade.apply(transactionInfo);
				transactionsSize += transactionInfo.pEntity->Size;
			}

			auto transactionHashes = test::ExtractHashes(transactionInfos);
			transactionHashes.push_back(Hash256()); // zero hash is used for block

			// Act: commit (update header size to match expected)
			auto pBlockHeader = CreateBlockHeaderWithHeight(Default_Height + Height(1));
			pBlockHeader->Size += transactionsSize;
			auto pBlock = facade.commit(*pBlockHeader);

			// Assert: transactions have been removed
			AssertEmpty(facade);

			// - check entity infos (hashes): validator and observer should be called for all notifications (2 per transaction and block)
			std::vector<std::pair<size_t, size_t>> expectedIndexIdPairs{
				{ 0, 1 }, { 0, 2 }, { 1, 1 }, { 1, 2 }, { 2, 1 }, { 2, 2 }, { 3, 1 }, { 3, 2 },
				{ 4, 1 }, { 4, 2 }
			};
			AssertEntityInfos("validator", executionConfig.pValidator->params(), transactionHashes, expectedIndexIdPairs);
			AssertEntityInfos("observer", executionConfig.pObserver->params(), transactionHashes, expectedIndexIdPairs);

			// - check contexts
			AssertValidatorContexts(executionConfig, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 });
			AssertObserverContexts(executionConfig, 10);

			// - check block
			EXPECT_EQ_MEMORY(pBlockHeader.get(), pBlock.get(), sizeof(model::BlockHeader));
			EXPECT_EQ(4u, model::CalculateBlockTransactionsInfo(*pBlock).Count);

			auto i = 0u;
			for (const auto& transaction : pBlock->Transactions()) {
				EXPECT_EQ(*transactionInfos[i].pEntity, transaction) << "transaction at " << i;
				++i;
			}
		});
	}

	// endregion

	// region FacadeTestContext

	namespace {
		struct FacadeTestContext {
		public:
			FacadeTestContext(
					const model::BlockChainConfiguration& config,
					const chain::ExecutionConfiguration& executionConfig,
					Height height)
					: m_config(config)
					, m_executionConfig(executionConfig)
					, m_height(height)
					, m_cache(test::CreateEmptyCatapultCache(m_config, CreateCacheConfiguration(m_dbDirGuard.name()))) {
				test::AddMarkerAccount(m_cache);
				setCacheHeight(Default_Height);
			}

		public:
			cache::CatapultCache& cache() {
				return m_cache;
			}

		public:
			void prepareSignerAccount(const Key& signer) {
				auto cacheDelta = m_cache.createDelta();

				auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();
				accountStateCacheDelta.addAccount(signer, Height(1));

				// set importance in order to mark account as high value (so that activity buckets influence state hash)
				auto importanceHeight = model::ConvertToImportanceHeight(m_height, m_config.ImportanceGrouping);
				auto accountStateIter = accountStateCacheDelta.find(signer);
				accountStateIter.get().ImportanceSnapshots.set(Importance(1), importanceHeight);

				// set buckets on half of accounts
				if (0 == signer[signer.size() - 1] % 2) {
					accountStateIter.get().ActivityBuckets.update(importanceHeight, [](auto& bucket) {
						bucket.TotalFeesPaid = Amount(1000);
					});
				}

				// recalculate the state hash and commit changes
				cacheDelta.calculateStateHash(Default_Height);
				m_cache.commit(Default_Height);
			}

			void prepareTransactionSignerAccounts(const std::vector<model::TransactionInfo>& transactionInfos) {
				for (const auto& transactionInfo : transactionInfos)
					prepareSignerAccount(transactionInfo.pEntity->SignerPublicKey);
			}

			void setCacheHeight(Height height) {
				auto delta = m_cache.createDelta();
				m_cache.commit(height);
			}

		public:
			std::unique_ptr<HarvestingUtFacade> createFacade() const {
				HarvestingUtFacadeFactory factory(m_cache, m_config, m_executionConfig);
				return factory.create(Default_Time);
			}

			std::unique_ptr<model::Block> generate(
					const model::BlockHeader& blockHeader,
					const std::vector<model::TransactionInfo>& transactionInfos,
					size_t numUnapplies = 0) const {
				auto pFacade = createFacade();
				if (!pFacade)
					return nullptr;

				for (const auto& transactionInfo : transactionInfos)
					pFacade->apply(transactionInfo);

				for (auto i = 0u; i < numUnapplies; ++i)
					pFacade->unapply();

				return pFacade->commit(blockHeader);
			}

		private:
			static cache::CacheConfiguration CreateCacheConfiguration(const std::string& databaseDirectory) {
				return cache::CacheConfiguration(databaseDirectory, utils::FileSize(), cache::PatriciaTreeStorageMode::Enabled);
			}

		private:
			test::TempDirectoryGuard m_dbDirGuard;
			model::BlockChainConfiguration m_config;
			chain::ExecutionConfiguration m_executionConfig;
			Height m_height;
			cache::CatapultCache m_cache;
		};
	}

	// endregion

	// region commit - verifiable settings honored

	namespace {
		std::vector<model::TransactionInfo> CreateTransactionInfosWithIncreasingMultipliers(uint32_t numTransactionInfos) {
			std::vector<std::pair<uint32_t, uint32_t>> sizeMultiplierPairs;
			for (auto i = 0u; i < numTransactionInfos; ++i)
				sizeMultiplierPairs.emplace_back(200, (2 + i) * 10);

			return test::CreateTransactionInfosFromSizeMultiplierPairs(sizeMultiplierPairs);
		}

		Hash256 CalculateEnabledTestReceiptsHash(uint32_t numReceipts) {
			model::BlockStatementBuilder statementBuilder;
			for (auto i = 0u; i < numReceipts; ++i) {
				// MockAggregateNotificationObserver simulates receipt source changes by incrementing primary source id for every
				// new entity (with sequence id 1). This is not identical to real behavior where blocks have id { 0, 0 }.
				if (0 == i % 2)
					statementBuilder.setSource({ i / 2 + 1, 0 });

				model::Receipt receipt{};
				receipt.Size = sizeof(model::Receipt);
				receipt.Type = static_cast<model::ReceiptType>(2 * (i + 1));
				statementBuilder.addReceipt(receipt);
			}

			return model::CalculateMerkleHash(*statementBuilder.build());
		}

		class TestStateHashCalculator {
		public:
			explicit TestStateHashCalculator(const cache::CatapultCache& cache)
					: m_cacheDetachableDelta(cache.createDetachableDelta())
					, m_cacheDetachedDelta(m_cacheDetachableDelta.detach())
					, m_pCacheDelta(m_cacheDetachedDelta.tryLock())
			{}

		public:
			void creditSurpluses(
					const std::vector<model::TransactionInfo>& transactionInfos,
					const std::vector<size_t>& indexToMultiplierMap = {}) {
				auto i = 1u;
				auto& accountStateCacheDelta = m_pCacheDelta->sub<cache::AccountStateCache>();
				for (auto& transactionInfo : transactionInfos) {
					auto accountStateIter = accountStateCacheDelta.find(transactionInfo.pEntity->SignerPublicKey);
					auto& accountState = accountStateIter.get();

					// apply fee surplus to balance and fees paid
					auto multiplier = indexToMultiplierMap.empty() ? i : indexToMultiplierMap[i - 1];
					auto surplus = Amount(multiplier * transactionInfo.pEntity->Size);
					accountState.Balances.credit(Currency_Mosaic_Id, surplus);
					accountState.ActivityBuckets.tryUpdate(accountState.ImportanceSnapshots.height(), [surplus](auto& bucket) {
						bucket.TotalFeesPaid = bucket.TotalFeesPaid - surplus;
					});

					++i;
				}
			}

			void addNewRecipientAccounts(const utils::KeySet& publicKeys, Height height) {
				auto& accountStateCacheDelta = m_pCacheDelta->sub<cache::AccountStateCache>();
				for (const auto& publicKey : publicKeys)
					accountStateCacheDelta.addAccount(publicKey, height);
			}

			Hash256 calculate() {
				return m_pCacheDelta->calculateStateHash(Default_Height + Height(1)).StateHash;
			}

		private:
			cache::CatapultCacheDetachableDelta m_cacheDetachableDelta;
			cache::CatapultCacheDetachedDelta m_cacheDetachedDelta;
			std::unique_ptr<cache::CatapultCacheDelta> m_pCacheDelta;
		};

		template<typename TAssertHashes>
		void RunEnabledTest(StateVerifyOptions verifyOptions, uint32_t numTransactions, TAssertHashes assertHashes) {
			// Arrange: prepare context
			test::MockExecutionConfiguration executionConfig;
			executionConfig.pObserver->enableReceiptGeneration();
			FacadeTestContext context(CreateBlockChainConfiguration(verifyOptions), executionConfig.Config, Default_Height + Height(1));

			auto pBlockHeader = CreateBlockHeaderWithHeight(Default_Height + Height(1));
			pBlockHeader->FeeMultiplier = BlockFeeMultiplier(1);
			context.prepareSignerAccount(pBlockHeader->SignerPublicKey);

			auto transactionInfos = CreateTransactionInfosWithIncreasingMultipliers(numTransactions);
			context.prepareTransactionSignerAccounts(transactionInfos);

			// Act: apply all transactions
			auto pBlock = context.generate(*pBlockHeader, transactionInfos);

			// Assert: check the block execution dependent hashes
			ASSERT_TRUE(!!pBlock);

			auto expectedReceiptsHash = CalculateEnabledTestReceiptsHash(2 * (numTransactions + 1));

			TestStateHashCalculator calculator(context.cache());
			calculator.creditSurpluses(transactionInfos);
			auto expectedStateHash = calculator.calculate();

			assertHashes(expectedReceiptsHash, expectedStateHash, *pBlock);
		}
	}

	TEST(TEST_CLASS, ZeroHashesAreReturnedWhenVerifiableReceiptsAndStateAreDisabled) {
		// Act:
		RunEnabledTest(StateVerifyOptions::None, 3, [](const auto&, const auto&, const auto& block) {
			// Assert:
			EXPECT_EQ(Hash256(), block.ReceiptsHash);
			EXPECT_EQ(Hash256(), block.StateHash);
		});
	}

	TEST(TEST_CLASS, NonzeroReceiptsHashIsReturnedWhenVerifiableReceiptsIsEnabled) {
		// Act:
		RunEnabledTest(StateVerifyOptions::Receipts, 3, [](const auto& expectedReceiptsHash, const auto&, const auto& block) {
			// Sanity:
			EXPECT_NE(Hash256(), expectedReceiptsHash);

			// Assert:
			EXPECT_EQ(expectedReceiptsHash, block.ReceiptsHash);
			EXPECT_EQ(Hash256(), block.StateHash);
		});
	}

	TEST(TEST_CLASS, NonzeroStateHashIsReturnedWhenVerifiableStateIsEnabled) {
		// Act:
		RunEnabledTest(StateVerifyOptions::State, 3, [](const auto&, const auto& expectedStateHash, const auto& block) {
			// Sanity:
			EXPECT_NE(Hash256(), expectedStateHash);

			// Assert:
			EXPECT_EQ(Hash256(), block.ReceiptsHash);
			EXPECT_EQ(expectedStateHash, block.StateHash);
		});
	}

	TEST(TEST_CLASS, NonzeroHashesAreReturnedWhenVerifiableReceiptsAndStateAreEnabled) {
		// Act:
		RunEnabledTest(StateVerifyOptions::All, 3, [](const auto& expectedReceiptsHash, const auto& expectedStateHash, const auto& block) {
			// Sanity:
			EXPECT_NE(Hash256(), expectedReceiptsHash);
			EXPECT_NE(Hash256(), expectedStateHash);

			// Assert:
			EXPECT_EQ(expectedReceiptsHash, block.ReceiptsHash);
			EXPECT_EQ(expectedStateHash, block.StateHash);
		});
	}

	// endregion

	// region commit - verifiable state correct after undo

	namespace {
		utils::KeySet ExtractDependentPublicKeys(const std::vector<model::TransactionInfo>& transactionInfos) {
			utils::KeySet publicKeys;
			for (const auto& transactionInfo : transactionInfos)
				publicKeys.insert(reinterpret_cast<const Key&>(transactionInfo.EntityHash));

			// add public key coerced from zero hash associated with block
			publicKeys.insert(Key());
			return publicKeys;
		}

		void AssertValidHashesAreReturnedAfterUnapplyOperation(const consumer<std::vector<model::TransactionInfo>&>& mutator) {
			// Arrange: prepare context
			test::MockExecutionConfiguration executionConfig;
			executionConfig.pObserver->enableReceiptGeneration();
			executionConfig.pObserver->enableRollbackEmulation();
			executionConfig.pNotificationPublisher->emulatePublicKeyNotifications();
			FacadeTestContext context(
					CreateBlockChainConfiguration(StateVerifyOptions::All),
					executionConfig.Config,
					Default_Height + Height(1));

			auto pBlockHeader = CreateBlockHeaderWithHeight(Default_Height + Height(1));
			pBlockHeader->FeeMultiplier = BlockFeeMultiplier(1);
			context.prepareSignerAccount(pBlockHeader->SignerPublicKey);

			auto transactionInfos = CreateTransactionInfosWithIncreasingMultipliers(5);
			mutator(transactionInfos);
			context.prepareTransactionSignerAccounts(transactionInfos);

			// Act: apply 5 transactions and unapply 2
			auto pBlock = context.generate(*pBlockHeader, transactionInfos, 2);

			// Assert: check the block execution dependent hashes
			ASSERT_TRUE(!!pBlock);

			std::vector<model::TransactionInfo> expectedTransactionInfos;
			for (auto i = 0u; i < 3; ++i)
				expectedTransactionInfos.push_back(transactionInfos[i].copy());

			auto expectedReceiptsHash = CalculateEnabledTestReceiptsHash(2 * (3 + 1));

			TestStateHashCalculator calculator(context.cache());
			calculator.creditSurpluses(expectedTransactionInfos);
			calculator.addNewRecipientAccounts(ExtractDependentPublicKeys(expectedTransactionInfos), Default_Height + Height(1));
			auto expectedStateHash = calculator.calculate();

			EXPECT_EQ(expectedReceiptsHash, pBlock->ReceiptsHash);
			EXPECT_EQ(expectedStateHash, pBlock->StateHash);
		}
	}

	TEST(TEST_CLASS, ValidHashesAreReturnedAfterUnapplyOperation) {
		AssertValidHashesAreReturnedAfterUnapplyOperation([](const auto&) {});
	}

	TEST(TEST_CLASS, ValidHashesAreReturnedAfterUnapplyOperationThatAffectsAppliedAccounts) {
		AssertValidHashesAreReturnedAfterUnapplyOperation([](auto& transactionInfos) {
			// Arrange: use alternating entity hashes
			auto i = 0u;
			for (auto& transactionInfo : transactionInfos)
				transactionInfo.EntityHash = transactionInfos[i++ % 2].EntityHash;
		});
	}

	TEST(TEST_CLASS, ValidHashesAreReturnedAfterApplyOperationWithSomeFailures) {
		// Arrange: prepare context
		test::MockExecutionConfiguration executionConfig;
		executionConfig.pObserver->enableReceiptGeneration();
		executionConfig.pObserver->enableRollbackEmulation();
		executionConfig.pNotificationPublisher->emulatePublicKeyNotifications();
		FacadeTestContext context(
				CreateBlockChainConfiguration(StateVerifyOptions::All),
				executionConfig.Config,
				Default_Height + Height(1));

		auto pBlockHeader = CreateBlockHeaderWithHeight(Default_Height + Height(1));
		pBlockHeader->FeeMultiplier = BlockFeeMultiplier(1);
		context.prepareSignerAccount(pBlockHeader->SignerPublicKey);

		auto transactionInfos = CreateTransactionInfosWithIncreasingMultipliers(5);
		context.prepareTransactionSignerAccounts(transactionInfos);

		// - mark two failures (triggers are intentionally different to maximize coverage)
		executionConfig.pValidator->setResult(validators::ValidationResult::Failure, transactionInfos[1].EntityHash, 2);
		executionConfig.pValidator->setResult(validators::ValidationResult::Failure, transactionInfos[3].EntityHash, 1);

		// Act: apply 5 transactions with two failures (but no undos)
		auto pBlock = context.generate(*pBlockHeader, transactionInfos, 0);

		// Assert: check the block execution dependent hashes
		ASSERT_TRUE(!!pBlock);

		std::vector<model::TransactionInfo> expectedTransactionInfos;
		for (auto i : { 0u, 2u, 4u })
			expectedTransactionInfos.push_back(transactionInfos[i].copy());

		auto expectedReceiptsHash = CalculateEnabledTestReceiptsHash(2 * (3 + 1));

		TestStateHashCalculator calculator(context.cache());
		calculator.creditSurpluses(expectedTransactionInfos, {
			// multipliers are configured as `index + 1`, but there are gaps because transactions at indexes 1 and 3 were dropped
			// remaining transactions have initial indexes of { 0, 2, 4 }, which implies multipliers of { 1, 3, 5 }
			1, 3, 5
		});
		calculator.addNewRecipientAccounts(ExtractDependentPublicKeys(expectedTransactionInfos), Default_Height + Height(1));
		auto expectedStateHash = calculator.calculate();

		EXPECT_EQ(expectedReceiptsHash, pBlock->ReceiptsHash);
		EXPECT_EQ(expectedStateHash, pBlock->StateHash);
	}

	// endregion

	// region commit - AccountState integration

	namespace {
		class AccountAwareMockNotificationPublisher : public test::MockNotificationPublisher {
		public:
			void publish(const model::WeakEntityInfo& entityInfo, model::NotificationSubscriber& subscriber) const override {
				if (mocks::MockTransaction::Entity_Type != entityInfo.type())
					return;

				const auto& mockTransaction = static_cast<const mocks::MockTransaction&>(entityInfo.entity());
				const auto& recipientPublicKey = mockTransaction.RecipientPublicKey;
				if (*mockTransaction.DataPtr()) {
					auto recipientAddress = model::PublicKeyToAddress(recipientPublicKey, Network_Identifier);
					subscriber.notify(model::AccountAddressNotification(recipientAddress));
				} else {
					subscriber.notify(model::AccountPublicKeyNotification(recipientPublicKey));
				}
			}
		};

		auto CreateMockTransactionInfo(const Key& signerPublicKey, const Key& recipientPublicKey, bool tagAddress) {
			auto pTransaction = mocks::CreateMockTransaction(1);
			pTransaction->SignerPublicKey = signerPublicKey;
			pTransaction->MaxFee = Amount(0);
			pTransaction->RecipientPublicKey = recipientPublicKey;
			*pTransaction->DataPtr() = tagAddress ? 1 : 0;

			auto transactionInfo = model::TransactionInfo(std::move(pTransaction));
			test::FillWithRandomData(transactionInfo.EntityHash);
			test::FillWithRandomData(transactionInfo.MerkleComponentHash);
			return transactionInfo;
		}

		template<typename TAction>
		void RunAppliedTransactionsWithNewAccountsTests(size_t numUnapplies, TAction action) {
			// Arrange: prepare context with custom NotificationPublisher so AccountStateCache behavior can be more finely targeted
			test::MockExecutionConfiguration executionConfig;
			executionConfig.pNotificationPublisher = std::make_shared<AccountAwareMockNotificationPublisher>();
			executionConfig.Config.pNotificationPublisher = executionConfig.pNotificationPublisher;
			FacadeTestContext context(
					CreateBlockChainConfiguration(StateVerifyOptions::All),
					executionConfig.Config,
					Default_Height + Height(1));

			auto pBlockHeader = CreateBlockHeaderWithHeight(Default_Height + Height(1));
			pBlockHeader->FeeMultiplier = BlockFeeMultiplier(0);
			context.prepareSignerAccount(pBlockHeader->SignerPublicKey);

			// Act: apply all, revert some, then commit
			// - prepare transactions such that signer A is known account but other recipients are new
			auto publicKeys = test::GenerateRandomDataVector<Key>(4);
			auto pFacade = context.createFacade();
			pFacade->apply(CreateMockTransactionInfo(pBlockHeader->SignerPublicKey, publicKeys[0], true)); //  A => B (Address)
			pFacade->apply(CreateMockTransactionInfo(pBlockHeader->SignerPublicKey, publicKeys[1], false)); // A => C (PublicKey)
			pFacade->apply(CreateMockTransactionInfo(pBlockHeader->SignerPublicKey, publicKeys[0], false)); // A => B (PublicKey)
			pFacade->apply(CreateMockTransactionInfo(pBlockHeader->SignerPublicKey, publicKeys[1], true)); //  A => C (Address)
			pFacade->apply(CreateMockTransactionInfo(pBlockHeader->SignerPublicKey, publicKeys[2], true)); //  A => D (Address)
			pFacade->apply(CreateMockTransactionInfo(pBlockHeader->SignerPublicKey, publicKeys[3], false)); // A => E (PublicKey)

			for (auto i = 0u; i < numUnapplies; ++i)
				pFacade->unapply();

			auto pBlock = pFacade->commit(*pBlockHeader);

			// Assert:
			ASSERT_TRUE(!!pBlock);

			auto cacheDelta = context.cache().createDelta();
			action(pBlock->StateHash, publicKeys, cacheDelta);
		}
	}

	TEST(TEST_CLASS, AccountStateCacheIsProperlyUpdatedWhenAppliedTransactionsAddNewAccounts_NoneUnapplied) {
		// Arrange: unapply none
		RunAppliedTransactionsWithNewAccountsTests(0, [](const auto& blockStateHash, const auto& publicKeys, auto& cacheDelta) {
			// Assert: check state hash
			auto height = Default_Height + Height(1);
			auto& accountStateCacheDelta = cacheDelta.template sub<cache::AccountStateCache>();
			accountStateCacheDelta.addAccount(publicKeys[0], height);
			accountStateCacheDelta.addAccount(publicKeys[1], height);
			accountStateCacheDelta.addAccount(model::PublicKeyToAddress(publicKeys[2], Network_Identifier), height);
			accountStateCacheDelta.addAccount(publicKeys[3], height);

			auto stateHashInfo = cacheDelta.calculateStateHash(height);
			EXPECT_EQ(stateHashInfo.StateHash, blockStateHash);
		});
	}

	TEST(TEST_CLASS, AccountStateCacheIsProperlyUpdatedWhenAppliedTransactionsAddNewAccounts_SomeUnapplied) {
		// Arrange: { D, E } fully
		RunAppliedTransactionsWithNewAccountsTests(2, [](const auto& blockStateHash, const auto& publicKeys, auto& cacheDelta) {
			// Assert: check state hash
			auto height = Default_Height + Height(1);
			auto& accountStateCacheDelta = cacheDelta.template sub<cache::AccountStateCache>();
			accountStateCacheDelta.addAccount(publicKeys[0], height);
			accountStateCacheDelta.addAccount(publicKeys[1], height);

			auto stateHashInfo = cacheDelta.calculateStateHash(height);
			EXPECT_EQ(stateHashInfo.StateHash, blockStateHash);
		});
	}

	TEST(TEST_CLASS, AccountStateCacheIsProperlyUpdatedWhenAppliedTransactionsAddNewAccounts_SomePartiallyUnapplied) {
		// Arrange: { D, E } fully and { B, C } partially
		RunAppliedTransactionsWithNewAccountsTests(4, [](const auto& blockStateHash, const auto& publicKeys, auto& cacheDelta) {
			// Assert: check state hash
			auto height = Default_Height + Height(1);
			auto& accountStateCacheDelta = cacheDelta.template sub<cache::AccountStateCache>();
			accountStateCacheDelta.addAccount(model::PublicKeyToAddress(publicKeys[0], Network_Identifier), height);
			accountStateCacheDelta.addAccount(publicKeys[1], height);

			auto stateHashInfo = cacheDelta.calculateStateHash(height);
			EXPECT_EQ(stateHashInfo.StateHash, blockStateHash);
		});
	}

	TEST(TEST_CLASS, AccountStateCacheIsProperlyUpdatedWhenAppliedTransactionsAddNewAccounts_AllUnapplied) {
		// Arrange: unapply all fully
		RunAppliedTransactionsWithNewAccountsTests(6, [](const auto& blockStateHash, const auto&, auto& cacheDelta) {
			// Assert: check state hash
			auto height = Default_Height + Height(1);

			auto stateHashInfo = cacheDelta.calculateStateHash(height);
			EXPECT_EQ(stateHashInfo.StateHash, blockStateHash);
		});
	}

	// endregion
}}
