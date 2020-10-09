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

#include "catapult/cache/CatapultCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/chain/BlockExecutor.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/extensions/PluginUtils.h"
#include "catapult/observers/NotificationObserverAdapter.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/ResolverTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS RootObserverIntegrityTests

	namespace {
		using NotifyMode = observers::NotifyMode;

		constexpr auto Harvesting_Mosaic_Id = MosaicId(9876);
		constexpr auto Network_Identifier = model::NetworkIdentifier::Private_Test;

		Importance GetTotalChainImportance(uint32_t numAccounts) {
			return Importance(numAccounts * (numAccounts + 1) / 2);
		}

		Amount GetTotalChainBalance(uint32_t numAccounts) {
			return Amount(GetTotalChainImportance(numAccounts).unwrap() * 1'000'000);
		}

		// region test context

		template<uint32_t Max_Rollback_Blocks>
		class TestContext {
		public:
			explicit TestContext(uint32_t numAccounts)
					: m_pPluginManager(test::CreatePluginManagerWithRealPlugins(test::CreatePrototypicalCatapultConfiguration(
							CreateBlockChainConfiguration(numAccounts),
							m_tempDataDir.name())))
					, m_cache(m_pPluginManager->createCache())
					, m_specialAccountPublicKey(test::GenerateRandomByteArray<Key>()) {
				config::CatapultDataDirectoryPreparer::Prepare(m_tempDataDir.name());

				// register mock transaction plugin so that BalanceTransferNotifications are produced and observed
				// (MockTransaction Publish XORs recipient address, so XOR address resolver is required
				// for proper roundtripping or else test will fail)
				m_pPluginManager->addTransactionSupport(mocks::CreateMockTransactionPlugin(mocks::PluginOptionFlags::Publish_Transfers));

				// seed the "nemesis" / transfer account (this account is used to fund all other accounts)
				auto delta = m_cache.createDelta();
				auto& accountStateCache = delta.template sub<cache::AccountStateCache>();
				accountStateCache.addAccount(m_specialAccountPublicKey, Height(1));
				auto& accountState = accountStateCache.find(m_specialAccountPublicKey).get();
				accountState.Balances.credit(Harvesting_Mosaic_Id, GetTotalChainBalance(numAccounts));
				m_cache.commit(Height());
			}

		public:
			// accounts are funded by "nemesis" account
			// (startAccountShortId with 1M * baseUnit, startAccountShortId + 1 with 2M * baseUnit, ...)
			void addAccounts(uint8_t startAccountShortId, uint8_t numAccounts, Height height, uint8_t baseUnit = 1) {
				auto& transactions = m_heightToTransactions[height];

				for (uint8_t i = startAccountShortId; i < startAccountShortId + numAccounts; ++i) {
					auto multiplier = static_cast<uint8_t>(i - startAccountShortId + 1);
					auto pTransaction = mocks::CreateTransactionWithFeeAndTransfers(Amount(), {
						{ test::UnresolveXor(Harvesting_Mosaic_Id), Amount(multiplier * baseUnit * 1'000'000) }
					});
					pTransaction->SignerPublicKey = m_specialAccountPublicKey;
					pTransaction->RecipientPublicKey = Key{ { i } };
					pTransaction->Network = Network_Identifier;
					transactions.push_back(std::move(pTransaction));
				}
			}

			// send entire balance of all accounts to "nemesis" account
			void zeroBalances(uint8_t startAccountShortId, uint8_t numAccounts, Height height) {
				auto& transactions = m_heightToTransactions[height];

				auto accountStateCacheView = m_cache.sub<cache::AccountStateCache>().createView();
				for (uint8_t i = startAccountShortId; i < startAccountShortId + numAccounts; ++i) {
					const auto& accountState = accountStateCacheView->find(Key{ { i } }).get();
					auto pTransaction = mocks::CreateTransactionWithFeeAndTransfers(Amount(), {
						{ test::UnresolveXor(Harvesting_Mosaic_Id), accountState.Balances.get(Harvesting_Mosaic_Id) }
					});
					pTransaction->SignerPublicKey = Key{ { i } };
					pTransaction->RecipientPublicKey = m_specialAccountPublicKey;
					pTransaction->Network = Network_Identifier;
					transactions.push_back(std::move(pTransaction));
				}
			}

			// send entire balance of accountShortId1 to accountShortId2
			void moveBalance(uint8_t accountShortId1, uint8_t accountShortId2, Height height) {
				auto& transactions = m_heightToTransactions[height];

				auto accountStateCacheView = m_cache.sub<cache::AccountStateCache>().createView();
				const auto& accountState1 = accountStateCacheView->find(Key{ { accountShortId1 } }).get();
				auto pTransaction = mocks::CreateTransactionWithFeeAndTransfers(Amount(), {
					{ test::UnresolveXor(Harvesting_Mosaic_Id), accountState1.Balances.get(Harvesting_Mosaic_Id) }
				});
				pTransaction->SignerPublicKey = accountState1.PublicKey;
				pTransaction->RecipientPublicKey = Key{ { accountShortId2 } };
				pTransaction->Network = Network_Identifier;
				transactions.push_back(std::move(pTransaction));
			}

		public:
			void notify(Height height, NotifyMode mode) {
				// Arrange:
				auto pBlock = createBlock(height);
				auto blockElement = test::BlockToBlockElement(*pBlock);

				// - use XOR resolvers because Publish_Transfers is enabled
				observers::NotificationObserverAdapter rootObserver(
						m_pPluginManager->createObserver(),
						m_pPluginManager->createNotificationPublisher());
				auto resolverContext = test::CreateResolverContextXor();

				auto delta = m_cache.createDelta();
				auto observerState = observers::ObserverState(delta);
				auto blockExecutionContext = chain::BlockExecutionContext(rootObserver, resolverContext, observerState);

				// Act: use BlockExecutor to execute all transactions and blocks
				if (NotifyMode::Commit == mode)
					chain::ExecuteBlock(blockElement, blockExecutionContext);
				else
					chain::RollbackBlock(blockElement, blockExecutionContext);

				delta.template sub<cache::AccountStateCache>().updateHighValueAccounts(height);
				m_cache.commit(height);
			}

			void notifyAllCommit(Height startHeight, Height endHeight) {
				for (auto height = startHeight; height <= endHeight; height = height + Height(1))
					notify(height, NotifyMode::Commit);
			}

			void notifyAllRollback(Height startHeight, Height endHeight) {
				for (auto height = startHeight; height >= endHeight; height = height - Height(1))
					notify(height, NotifyMode::Rollback);
			}

		public:
			struct AssertOptions {
			public:
				AssertOptions(
						uint8_t startAccountShortId,
						uint8_t numAccounts,
						model::ImportanceHeight importanceHeight,
						uint8_t startAdjustment = 0)
						: StartAccountShortId(startAccountShortId)
						, NumAccounts(numAccounts)
						, ImportanceHeight(importanceHeight)
						, StartAdjustment(startAdjustment)
				{}

			public:
				uint8_t StartAccountShortId;
				uint8_t NumAccounts;
				model::ImportanceHeight ImportanceHeight;
				uint8_t StartAdjustment;
			};

			void assertSingleImportance(
					uint8_t accountShortId,
					model::ImportanceHeight expectedImportanceHeight,
					Importance expectedImportance) {
				const auto message = "importance for account " + std::to_string(accountShortId);
				auto accountStateCacheView = m_cache.sub<cache::AccountStateCache>().createView();

				// tests only calculate importance once, therefore only the raw score in the bucket will be non-zero
				const auto& accountState = accountStateCacheView->find(Key{ { accountShortId } }).get();
				EXPECT_EQ(expectedImportanceHeight, accountState.ImportanceSnapshots.height()) << message;
				EXPECT_EQ(expectedImportance, Importance(accountState.ActivityBuckets.get(expectedImportanceHeight).RawScore)) << message;
			}

			void assertLinearImportances(const AssertOptions& options) {
				assertImportances(options, [](auto multiplier) {
					return Importance(multiplier);
				});
			}

			void assertZeroedImportances(const AssertOptions& options) {
				assertImportances(options, [](auto) {
					return Importance(0);
				});
			}

			void assertRemovedAccounts(uint8_t startAccountShortId, uint8_t numAccounts) {
				auto accountStateCacheView = m_cache.sub<cache::AccountStateCache>().createView();
				for (uint8_t i = startAccountShortId; i < startAccountShortId + numAccounts; ++i)
					EXPECT_FALSE(accountStateCacheView->contains(Key{ { i } })) << "importance for account " << static_cast<int>(i);
			}

		private:
			std::unique_ptr<model::Block> createBlock(Height height) {
				// if there are transactions, add them to the block
				auto transactionsIter = m_heightToTransactions.find(height);
				auto pBlock = m_heightToTransactions.end() == transactionsIter
						? test::GenerateEmptyRandomBlock()
						: test::GenerateBlockWithTransactions(transactionsIter->second);
				pBlock->Height = height;
				pBlock->FeeMultiplier = BlockFeeMultiplier(0);
				pBlock->BeneficiaryAddress = Address();

				// in order to emulate correctly, block must have same signer when executed and reverted
				auto signerIter = m_heightToBlockSigner.find(height);
				if (m_heightToBlockSigner.cend() == signerIter)
					m_heightToBlockSigner.emplace(height, pBlock->SignerPublicKey); // save signer used during commit
				else
					pBlock->SignerPublicKey = signerIter->second;

				return pBlock;
			}

			void assertImportances(const AssertOptions& options, const std::function<Importance (uint8_t)>& getImportanceFromMultiplier) {
				for (uint8_t i = options.StartAccountShortId; i < options.StartAccountShortId + options.NumAccounts; ++i) {
					auto multiplier = static_cast<uint8_t>(options.StartAdjustment + i - options.StartAccountShortId + 1);
					assertSingleImportance(i, options.ImportanceHeight, getImportanceFromMultiplier(multiplier));
				}
			}

		private:
			static model::BlockChainConfiguration CreateBlockChainConfiguration(uint32_t numAccounts) {
				auto config = model::BlockChainConfiguration::Uninitialized();
				config.Network.Identifier = Network_Identifier;
				config.HarvestingMosaicId = Harvesting_Mosaic_Id;
				config.ImportanceGrouping = 123;
				config.MaxDifficultyBlocks = 123;
				config.MaxRollbackBlocks = Max_Rollback_Blocks;
				config.TotalChainImportance = GetTotalChainImportance(numAccounts);
				config.MinHarvesterBalance = Amount(1'000'000);
				return config;
			}

		private:
			test::TempDirectoryGuard m_tempDataDir;
			std::shared_ptr<plugins::PluginManager> m_pPluginManager;
			cache::CatapultCache m_cache;

			Key m_specialAccountPublicKey;

			// undo tests require same block signer at heights (because HarvestFeeObserver needs to debit an existing account)
			std::unordered_map<Height, Key, utils::BaseValueHasher<Height>> m_heightToBlockSigner;
			std::unordered_map<Height, test::MutableTransactions, utils::BaseValueHasher<Height>> m_heightToTransactions;
		};

		// endregion
	}

	// region traits

#define ROLLBACK_TEST(TEST_NAME) \
	template<typename TTestContext> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_InfiniteRollback) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TestContext<0>>(); } \
	TEST(TEST_CLASS, TEST_NAME##_FiniteRollback) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<TestContext<124>>(); } \
	template<typename TTestContext> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region execute

	ROLLBACK_TEST(ExecuteCalculatesImportancesCorrectly) {
		// Arrange: create a context with 10/20 important accounts
		TTestContext context(10);
		context.addAccounts(1, 10, Height(246));
		context.addAccounts(25, 10, Height(246), 0);

		// Act: calculate importances at height 246
		context.notify(Height(246), NotifyMode::Commit);

		// Assert: importance should have been calculated from only high value account balances
		context.assertLinearImportances({ 1, 10, model::ImportanceHeight(246) }); // updated
		context.assertZeroedImportances({ 25, 10, model::ImportanceHeight(0) }); // excluded
	}

	ROLLBACK_TEST(ExecuteCalculatesImportancesCorrectly_WhenAllHighValueAccountsChangeAtImportanceHeight) {
		// Arrange: create a context with 10/10 important accounts
		TTestContext context(10);
		context.addAccounts(1, 10, Height(246));

		// - calculate importances at height 246
		context.notify(Height(246), NotifyMode::Commit);

		// - replace existing high value accounts with new high value accounts
		context.zeroBalances(1, 10, Height(247));
		context.addAccounts(25, 10, Height(247));

		// Act: calculate importances at height 369
		context.notifyAllCommit(Height(247), Height(369));

		// Assert: only newly high balance accounts should have updated importances
		context.assertLinearImportances({ 1, 10, model::ImportanceHeight(246) });
		context.assertLinearImportances({ 25, 10, model::ImportanceHeight(369) }); // updated
	}

	ROLLBACK_TEST(ExecuteCalculatesImportancesCorrectly_WhenSomeHighValueAccountsChangeAtImportanceHeight) {
		// Arrange: create a context with 10/10 important accounts
		TTestContext context(10);
		context.addAccounts(1, 10, Height(246));

		// - calculate importances at height 246
		context.notify(Height(246), NotifyMode::Commit);

		// - replace some existing high value accounts with new high value accounts
		context.zeroBalances(1, 5, Height(247));
		context.addAccounts(25, 5, Height(247));
		context.addAccounts(30, 5, Height(247), 0);

		// Act: calculate importances at height 369
		context.notifyAllCommit(Height(247), Height(369));

		// Assert: only current high balance accounts should have updated importances
		// - original accounts [6, 10] but NOT [1, 5]
		// - new accounts [25, 29] but NOT [30, 34]
		context.assertLinearImportances({ 1, 5, model::ImportanceHeight(246) });
		context.assertLinearImportances({ 6, 5, model::ImportanceHeight(369), 5 }); // updated
		context.assertLinearImportances({ 25, 5, model::ImportanceHeight(369) }); // updated
		context.assertZeroedImportances({ 30, 5, model::ImportanceHeight(0), 5 }); // excluded
	}

	// endregion

	// region undo one level

	ROLLBACK_TEST(UndoCalculatesImportancesCorrectly) {
		// Arrange: create a context with 10/20 important accounts
		TTestContext context(10);
		context.addAccounts(1, 10, Height(245));
		context.addAccounts(25, 10, Height(245), 0);

		// - calculate importances at height 245 (importance height will be 123)
		context.notify(Height(245), NotifyMode::Commit);

		// - change balances of two accounts and recalculate importances at height 246 (this ensures importances are different)
		context.moveBalance(1, 2, Height(246));
		context.notify(Height(246), NotifyMode::Commit);

		// Sanity:
		context.assertSingleImportance(1, model::ImportanceHeight(123), Importance(1)); // excluded
		context.assertSingleImportance(2, model::ImportanceHeight(246), Importance(3)); // increased (3 instead of 2)

		// Act: rollback importances to height 245
		context.notify(Height(246), NotifyMode::Rollback);

		// Assert: importance should have been restored for only high value account balances (balance changes are rolled back)
		context.assertLinearImportances({ 1, 10, model::ImportanceHeight(123) }); // reverted
		context.assertZeroedImportances({ 25, 10, model::ImportanceHeight(0) }); // excluded
	}

	ROLLBACK_TEST(UndoCalculatesImportancesCorrectly_WhenAllHighValueAccountsChangeAtImportanceHeight) {
		// Arrange: create a context with 10/10 important accounts
		TTestContext context(10);
		context.addAccounts(1, 10, Height(246));

		// - calculate importances at height 246
		context.notify(Height(246), NotifyMode::Commit);

		// - replace existing high value accounts with new high value accounts
		context.zeroBalances(1, 10, Height(247));
		context.addAccounts(25, 10, Height(247));

		// - calculate importances at height 369
		context.notifyAllCommit(Height(247), Height(369));

		// Sanity:
		context.assertLinearImportances({ 1, 10, model::ImportanceHeight(246) });
		context.assertLinearImportances({ 25, 10, model::ImportanceHeight(369) }); // updated

		// Act: rollback importances to height 246
		context.notifyAllRollback(Height(369), Height(247));

		// Assert: only pre-existing high balance accounts should have updated importances
		context.assertLinearImportances({ 1, 10, model::ImportanceHeight(246) });
		context.assertRemovedAccounts(25, 10); // reverted
	}

	ROLLBACK_TEST(UndoCalculatesImportancesCorrectly_WhenSomeHighValueAccountsChangeAtImportanceHeight) {
		// Arrange: create a context with 10/10 important accounts
		TTestContext context(10);
		context.addAccounts(1, 10, Height(246));

		// - calculate importances at height 246
		context.notify(Height(246), NotifyMode::Commit);

		// - replace some existing high value accounts with new high value accounts
		context.zeroBalances(1, 5, Height(247));
		context.addAccounts(25, 5, Height(247));
		context.addAccounts(30, 5, Height(247), 0);

		// - calculate importances at height 369
		context.notifyAllCommit(Height(247), Height(369));

		// Sanity:
		context.assertLinearImportances({ 1, 5, model::ImportanceHeight(246) });
		context.assertLinearImportances({ 6, 5, model::ImportanceHeight(369), 5 }); // updated
		context.assertLinearImportances({ 25, 5, model::ImportanceHeight(369) }); // updated
		context.assertZeroedImportances({ 30, 5, model::ImportanceHeight(0), 5 }); // excluded

		// Act: rollback importances to height 246
		context.notifyAllRollback(Height(369), Height(247));

		// Assert: only pre-existing high balance accounts should have updated importances
		context.assertLinearImportances({ 1, 5, model::ImportanceHeight(246) });
		context.assertLinearImportances({ 6, 5, model::ImportanceHeight(246), 5 }); // reverted
		context.assertRemovedAccounts(25, 5); // reverted
		context.assertRemovedAccounts(30, 5); // reverted
	}

	// endregion

	// region undo multiple levels

	ROLLBACK_TEST(UndoCalculatesImportancesCorrectly_WhenSomeHighValueAccountsChangeAtImportanceHeight_MultipleRollbacks) {
		// Arrange: create a context with 10/10 important accounts
		TTestContext context(10);
		context.addAccounts(1, 10, Height(122));

		// - calculate importances at heights 1 and 123
		//   (need to calculate at height 1 so that importances are nonzero at height 123 because effective importances are
		//   min of previous and current importances)
		context.notifyAllCommit(Height(122), Height(245));

		// - change balances of two accounts and recalculate importances at height 246 (this ensures importances are different)
		context.moveBalance(1, 2, Height(246));
		context.notify(Height(246), NotifyMode::Commit);

		// Sanity:
		context.assertSingleImportance(1, model::ImportanceHeight(123), Importance(1)); // excluded
		context.assertSingleImportance(2, model::ImportanceHeight(246), Importance(3)); // increased (3 instead of 2)

		// Arrange: replace some existing high value accounts with new high value accounts
		context.zeroBalances(1, 5, Height(247));
		context.addAccounts(25, 5, Height(247));
		context.addAccounts(30, 5, Height(247), 0);

		// - calculate importances at height 369
		context.notifyAllCommit(Height(247), Height(369));

		// Sanity:
		context.assertLinearImportances({ 6, 5, model::ImportanceHeight(369), 5 }); // updated
		context.assertLinearImportances({ 25, 5, model::ImportanceHeight(369) }); // updated

		// Act: rollback importances to height 245 (requires two rollbacks)
		context.notifyAllRollback(Height(369), Height(246));

		// Assert: only original high balance accounts should have restored importances
		// - important accounts at 369: 6 - 10, 25 - 29
		// - important accounts at 246: 2 - 10
		// - important accounts at 123: 1 - 10
		context.assertLinearImportances({ 1, 10, model::ImportanceHeight(123) }); // restored
		context.assertRemovedAccounts(25, 10); // reverted
	}

	ROLLBACK_TEST(UndoCalculatesImportancesCorrectly_WhenSomeHighValueAccountsChangeAtImportanceHeight_MultipleRollbacksToOriginal) {
		// Arrange: create a context with 10/10 important accounts
		TTestContext context(10);
		context.addAccounts(1, 10, Height(1));

		// - calculate importances at height 1
		context.notify(Height(1), NotifyMode::Commit);

		// - replace some existing high value accounts with new high value accounts
		context.zeroBalances(1, 5, Height(2));
		context.addAccounts(25, 5, Height(2));
		context.addAccounts(30, 5, Height(2), 0);

		//- calculate importances at height 245
		context.notifyAllCommit(Height(2), Height(245));

		// Sanity:
		context.assertLinearImportances({ 6, 5, model::ImportanceHeight(123), 5 }); // updated
		context.assertLinearImportances({ 25, 5, model::ImportanceHeight(123) }); // updated

		// Arrange: change balances of two accounts and recalculate importances at height 246 (this ensures importances are different)
		context.moveBalance(25, 26, Height(246));
		context.notify(Height(246), NotifyMode::Commit);

		// Sanity:
		context.assertSingleImportance(25, model::ImportanceHeight(123), Importance(1)); // excluded
		context.assertSingleImportance(26, model::ImportanceHeight(246), Importance(3)); // increased (3 instead of 2)

		// Act: rollback importances to height 122 (requires two rollbacks)
		context.notifyAllRollback(Height(246), Height(123));

		// Assert: only original high balance accounts should have restored importances
		// - important accounts at 246: 6 - 10, 26 - 29
		// - important accounts at 123: 6 - 10, 25 - 29
		// - important accounts at   1: 1 - 10
		context.assertLinearImportances({ 1, 10, model::ImportanceHeight(1) }); // restored
		context.assertZeroedImportances({ 25, 10, model::ImportanceHeight(0) }); // excluded
	}

	// endregion

	// region undo deep

	TEST(TEST_CLASS, UndoCalculatesDeepRollbacksCorrectly_InfiniteRollback) {
		// Arrange: create a context with 10/10 important accounts
		TestContext<0> context(10);
		context.addAccounts(1, 10, Height(1));

		// - calculate importances through height 1000
		context.notifyAllCommit(Height(1), Height(1000));

		// Act: rollback importances to height 122
		context.notifyAllRollback(Height(1000), Height(122));

		// Assert: importances are correct
		context.assertLinearImportances({ 1, 10, model::ImportanceHeight(1) });
	}

	TEST(TEST_CLASS, UndoDoesNotCalculateDeepRollbacksCorrectly_FiniteRollback) {
		// Arrange: create a context with 10/10 important accounts
		TestContext<124> context(10);
		context.addAccounts(1, 10, Height(1));

		// - calculate importances through height 1000
		context.notifyAllCommit(Height(1), Height(1000));

		// Act: rollback importances to height 122
		context.notifyAllRollback(Height(1000), Height(122));

		// Assert: importances are zeroed because rollback was too deep
		context.assertZeroedImportances({ 1, 10, model::ImportanceHeight(0) });
	}

	// endregion
}}
