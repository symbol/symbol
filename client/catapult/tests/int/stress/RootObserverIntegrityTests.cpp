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
#include "catapult/extensions/PluginUtils.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/local/LocalTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace extensions {

#define TEST_CLASS RootObserverIntegrityTests

	namespace {
		using NotifyMode = observers::NotifyMode;

		Amount GetTotalChainBalance(uint32_t numAccounts) {
			return Amount(numAccounts * (numAccounts + 1) / 2 * 1'000'000);
		}

		model::BlockChainConfiguration CreateBlockChainConfiguration(uint32_t numAccounts) {
			auto config = model::BlockChainConfiguration::Uninitialized();
			config.Network.Identifier = model::NetworkIdentifier::Mijin_Test;
			config.ImportanceGrouping = 123;
			config.MaxDifficultyBlocks = 123;
			config.MaxRollbackBlocks = 124;
			config.BlockPruneInterval = 360;
			config.TotalChainBalance = GetTotalChainBalance(numAccounts);
			config.MinHarvesterBalance = Amount(1'000'000);
			return config;
		}

		class TestContext {
		public:
			explicit TestContext(uint32_t numAccounts)
					: m_pPluginManager(test::CreatePluginManager(CreateBlockChainConfiguration(numAccounts)))
					, m_cache(m_pPluginManager->createCache())
					, m_specialAccountKey(test::GenerateRandomData<Key_Size>()) {
				// register mock transaction plugin so that BalanceTransferNotifications are produced and observed
				m_pPluginManager->addTransactionSupport(mocks::CreateMockTransactionPlugin(mocks::PluginOptionFlags::Publish_Transfers));

				// seed the "nemesis" / transfer account (this account is used to fund all other accounts)
				auto delta = m_cache.createDelta();
				auto& accountStateCache = delta.sub<cache::AccountStateCache>();
				accountStateCache.addAccount(m_specialAccountKey, Height(1));
				auto& accountState = accountStateCache.find(m_specialAccountKey).get();
				accountState.Balances.credit(Xem_Id, GetTotalChainBalance(numAccounts));
				m_cache.commit(Height());
			}

		public:
			// accounts are funded by "nemesis" account (startAccountId with 1M * baseUnit, startAccountId + 1 with 2M * baseUnit, ...)
			void addAccounts(uint8_t startAccountId, uint8_t numAccounts, Height height, uint8_t baseUnit = 1) {
				auto& transactions = m_heightToTransactions[height];

				for (uint8_t i = startAccountId; i < startAccountId + numAccounts; ++i) {
					uint8_t multiplier = i - startAccountId + 1;
					auto pTransaction = mocks::CreateTransactionWithFeeAndTransfers(Amount(), {
						{ Xem_Id, Amount(multiplier * baseUnit * 1'000'000) }
					});
					pTransaction->Signer = m_specialAccountKey;
					pTransaction->Recipient = Key{ { i } };
					transactions.push_back(std::move(pTransaction));
				}
			}

			// send entire balance of all accounts to "nemesis" account
			void zeroBalances(uint8_t startAccountId, uint8_t numAccounts, Height height) {
				auto& transactions = m_heightToTransactions[height];

				auto accountStateCacheView = m_cache.sub<cache::AccountStateCache>().createView();
				for (uint8_t i = startAccountId; i < startAccountId + numAccounts; ++i) {
					const auto& accountState = accountStateCacheView->find(Key{ { i } }).get();
					auto pTransaction = mocks::CreateTransactionWithFeeAndTransfers(Amount(), {
						{ Xem_Id, accountState.Balances.get(Xem_Id) }
					});
					pTransaction->Signer = Key{ { i } };
					pTransaction->Recipient = m_specialAccountKey;
					transactions.push_back(std::move(pTransaction));
				}
			}

			// send entire balance of accountId1 to accountId2
			void moveBalance(uint8_t accountId1, uint8_t accountId2, Height height) {
				auto& transactions = m_heightToTransactions[height];

				auto accountStateCacheView = m_cache.sub<cache::AccountStateCache>().createView();
				const auto& accountState1 = accountStateCacheView->find(Key{ { accountId1 } }).get();
				auto pTransaction = mocks::CreateTransactionWithFeeAndTransfers(Amount(), {
					{ Xem_Id, accountState1.Balances.get(Xem_Id) }
				});
				pTransaction->Signer = accountState1.PublicKey;
				pTransaction->Recipient = Key{ { accountId2 } };
				transactions.push_back(std::move(pTransaction));
			}

		public:
			void notify(Height height, NotifyMode mode) {
				// Arrange:
				auto pBlock = createBlock(height);
				auto blockElement = test::BlockToBlockElement(*pBlock);

				auto pRootObserver = CreateEntityObserver(*m_pPluginManager);

				auto delta = m_cache.createDelta();
				auto observerState = observers::ObserverState(delta, m_state);

				// Act: use BlockExecutor to execute all transactions and blocks
				if (NotifyMode::Commit == mode)
					chain::ExecuteBlock(blockElement, *pRootObserver, observerState);
				else
					chain::RollbackBlock(blockElement, *pRootObserver, observerState);

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
						uint8_t startAccountId,
						uint8_t numAccounts,
						model::ImportanceHeight importanceHeight,
						uint8_t startAdjustment = 0)
						: StartAccountId(startAccountId)
						, NumAccounts(numAccounts)
						, ImportanceHeight(importanceHeight)
						, StartAdjustment(startAdjustment)
				{}

			public:
				uint8_t StartAccountId;
				uint8_t NumAccounts;
				model::ImportanceHeight ImportanceHeight;
				uint8_t StartAdjustment;
			};

			void assertSingleImportance(
					uint8_t accountId,
					model::ImportanceHeight expectedImportanceHeight,
					Importance expectedImportance) {
				const auto message = "importance for account " + std::to_string(accountId);
				auto accountStateCacheView = m_cache.sub<cache::AccountStateCache>().createView();

				const auto& accountState = accountStateCacheView->find(Key{ { accountId } }).get();
				EXPECT_EQ(expectedImportanceHeight, accountState.ImportanceInfo.height()) << message;
				EXPECT_EQ(expectedImportance, accountState.ImportanceInfo.current()) << message;
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

			void assertRemovedAccounts(uint8_t startAccountId, uint8_t numAccounts) {
				auto accountStateCacheView = m_cache.sub<cache::AccountStateCache>().createView();
				for (uint8_t i = startAccountId; i < startAccountId + numAccounts; ++i)
					EXPECT_FALSE(accountStateCacheView->contains(Key{ { i } })) << "importance for account " << static_cast<int>(i);
			}

		private:
			std::unique_ptr<model::Block> createBlock(Height height) {
				// if there are transactions, add them to the block
				auto transactionsIter = m_heightToTransactions.find(height);
				auto pBlock = m_heightToTransactions.end() == transactionsIter
						? test::GenerateEmptyRandomBlock()
						: test::GenerateRandomBlockWithTransactions(transactionsIter->second);
				pBlock->Height = height;

				// in order to emulate correctly, block must have same signer when executed and reverted
				auto signerIter = m_heightToBlockSigner.find(height);
				if (m_heightToBlockSigner.cend() == signerIter)
					m_heightToBlockSigner.emplace(height, pBlock->Signer); // save signer used during commit
				else
					pBlock->Signer = signerIter->second;

				return pBlock;
			}

			void assertImportances(const AssertOptions& options, const std::function<Importance (uint8_t)>& getImportanceFromMultiplier) {
				for (uint8_t i = options.StartAccountId; i < options.StartAccountId + options.NumAccounts; ++i) {
					uint8_t multiplier = options.StartAdjustment + i - options.StartAccountId + 1;
					assertSingleImportance(i, options.ImportanceHeight, getImportanceFromMultiplier(multiplier));
				}
			}

		private:
			std::shared_ptr<plugins::PluginManager> m_pPluginManager;
			cache::CatapultCache m_cache;
			state::CatapultState m_state;

			Key m_specialAccountKey;

			// undo tests require same block signer at heights (because HarvestFeeObserver needs to debit an existing account)
			std::unordered_map<Height, Key, utils::BaseValueHasher<Height>> m_heightToBlockSigner;
			std::unordered_map<Height, test::MutableTransactions, utils::BaseValueHasher<Height>> m_heightToTransactions;
		};
	}

	// region execute

	TEST(TEST_CLASS, ExecuteCalculatesImportancesCorrectly) {
		// Arrange: create a context with 10/20 important accounts
		TestContext context(10);
		context.addAccounts(1, 10, Height(246));
		context.addAccounts(25, 10, Height(246), 0);

		// Act: calculate importances at height 246
		context.notify(Height(246), NotifyMode::Commit);

		// Assert: importance should have been calculated from only high value account balances
		context.assertLinearImportances({ 1, 10, model::ImportanceHeight(246) }); // updated
		context.assertZeroedImportances({ 25, 10, model::ImportanceHeight(0) }); // excluded
	}

	TEST(TEST_CLASS, ExecuteCalculatesImportancesCorrectly_WhenAllHighValueAccountsChangeAtImportanceHeight) {
		// Arrange: create a context with 10/10 important accounts
		TestContext context(10);
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

	TEST(TEST_CLASS, ExecuteCalculatesImportancesCorrectly_WhenSomeHighValueAccountsChangeAtImportanceHeight) {
		// Arrange: create a context with 10/10 important accounts
		TestContext context(10);
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

	TEST(TEST_CLASS, UndoCalculatesImportancesCorrectly) {
		// Arrange: create a context with 10/20 important accounts
		TestContext context(10);
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

	TEST(TEST_CLASS, UndoCalculatesImportancesCorrectly_WhenAllHighValueAccountsChangeAtImportanceHeight) {
		// Arrange: create a context with 10/10 important accounts
		TestContext context(10);
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

	TEST(TEST_CLASS, UndoCalculatesImportancesCorrectly_WhenSomeHighValueAccountsChangeAtImportanceHeight) {
		// Arrange: create a context with 10/10 important accounts
		TestContext context(10);
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

	TEST(TEST_CLASS, UndoCalculatesImportancesCorrectly_WhenSomeHighValueAccountsChangeAtImportanceHeight_MultipleRollbacks) {
		// Arrange: create a context with 10/10 important accounts
		TestContext context(10);
		context.addAccounts(1, 10, Height(245));

		// - calculate importances at height 245
		context.notify(Height(245), NotifyMode::Commit);

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

	TEST(TEST_CLASS, UndoCalculatesImportancesCorrectly_WhenSomeHighValueAccountsChangeAtImportanceHeight_MultipleRollbacksToOriginal) {
		// Arrange: create a context with 10/10 important accounts
		TestContext context(10);
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
}}
