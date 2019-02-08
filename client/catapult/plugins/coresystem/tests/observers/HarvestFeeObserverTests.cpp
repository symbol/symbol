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

#include "src/observers/Observers.h"
#include "tests/test/cache/BalanceTransferTestUtils.h"
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/plugins/AccountObserverTestContext.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

#define TEST_CLASS HarvestFeeObserverTests

	DEFINE_COMMON_OBSERVER_TESTS(HarvestFee, MosaicId())

	// region traits

	namespace {
		constexpr MosaicId Currency_Mosaic_Id(1234);

		struct UnlinkedAccountTraits {
			static auto AddAccount(cache::AccountStateCacheDelta& delta, const Key& publicKey, Height height) {
				delta.addAccount(publicKey, height);
				return delta.find(publicKey);
			}
		};

		struct MainAccountTraits {
			static auto AddAccount(cache::AccountStateCacheDelta& delta, const Key& publicKey, Height height) {
				// explicitly mark the account as a main account (local harvesting when remote harvesting is enabled)
				auto accountStateIter = UnlinkedAccountTraits::AddAccount(delta, publicKey, height);
				accountStateIter.get().AccountType = state::AccountType::Main;
				accountStateIter.get().LinkedAccountKey = test::GenerateRandomData<Key_Size>();
				return accountStateIter;
			}
		};

		struct RemoteAccountTraits {
			static auto AddAccount(cache::AccountStateCacheDelta& delta, const Key& publicKey, Height height) {
				// 1. add the main account with a balance
				auto mainAccountPublicKey = test::GenerateRandomData<Key_Size>();
				auto mainAccountStateIter = UnlinkedAccountTraits::AddAccount(delta, mainAccountPublicKey, height);
				mainAccountStateIter.get().AccountType = state::AccountType::Main;
				mainAccountStateIter.get().LinkedAccountKey = publicKey;

				// 2. add the remote account with specified key
				auto accountStateIter = UnlinkedAccountTraits::AddAccount(delta, publicKey, height);
				accountStateIter.get().AccountType = state::AccountType::Remote;
				accountStateIter.get().LinkedAccountKey = mainAccountPublicKey;
				return mainAccountStateIter;
			}
		};
	}

#define ACCOUNT_TYPE_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_UnlinkedAccount) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<UnlinkedAccountTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MainAccount) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MainAccountTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_RemoteAccount) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RemoteAccountTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region fee credit/debit

	namespace {
		template<typename TAction>
		void RunHarvestFeeObserverTest(NotifyMode notifyMode, TAction action) {
			// Arrange:
			test::AccountObserverTestContext context(notifyMode);

			auto pObserver = CreateHarvestFeeObserver(Currency_Mosaic_Id);

			// Act + Assert:
			action(context, *pObserver);
		}
	}

	ACCOUNT_TYPE_TRAITS_BASED_TEST(CommitCreditsHarvester) {
		// Arrange:
		RunHarvestFeeObserverTest(NotifyMode::Commit, [](test::AccountObserverTestContext& context, const auto& observer) {
			auto signerPublicKey = test::GenerateRandomData<Key_Size>();
			auto accountStateIter = TTraits::AddAccount(context.cache().sub<cache::AccountStateCache>(), signerPublicKey, Height(1));
			accountStateIter.get().Balances.credit(Currency_Mosaic_Id, Amount(987));

			auto notification = test::CreateBlockNotification(signerPublicKey);
			notification.TotalFee = Amount(123);

			// Act:
			test::ObserveNotification(observer, notification, context);

			// Assert:
			test::AssertBalances(context.cache(), accountStateIter.get().PublicKey, { { Currency_Mosaic_Id, Amount(987 + 123) } });

			// - if signer is remote, it should have an unchanged balance
			if (signerPublicKey != accountStateIter.get().PublicKey)
				test::AssertBalances(context.cache(), signerPublicKey, {});

			// - check receipt
			auto pStatement = context.statementBuilder().build();
			ASSERT_EQ(1u, pStatement->TransactionStatements.size());
			const auto& receiptPair = *pStatement->TransactionStatements.find(model::ReceiptSource());
			ASSERT_EQ(1u, receiptPair.second.size());

			const auto& receipt = static_cast<const model::BalanceChangeReceipt&>(receiptPair.second.receiptAt(0));
			ASSERT_EQ(sizeof(model::BalanceChangeReceipt), receipt.Size);
			EXPECT_EQ(1u, receipt.Version);
			EXPECT_EQ(model::Receipt_Type_Harvest_Fee, receipt.Type);
			EXPECT_EQ(accountStateIter.get().PublicKey, receipt.Account);
			EXPECT_EQ(Currency_Mosaic_Id, receipt.MosaicId);
			EXPECT_EQ(Amount(123), receipt.Amount);
		});
	}

	ACCOUNT_TYPE_TRAITS_BASED_TEST(RollbackDebitsHarvester) {
		// Arrange:
		RunHarvestFeeObserverTest(NotifyMode::Rollback, [](test::AccountObserverTestContext& context, const auto& observer) {
			auto signerPublicKey = test::GenerateRandomData<Key_Size>();
			auto accountStateIter = TTraits::AddAccount(context.cache().sub<cache::AccountStateCache>(), signerPublicKey, Height(1));
			accountStateIter.get().Balances.credit(Currency_Mosaic_Id, Amount(987 + 123));

			auto notification = test::CreateBlockNotification(signerPublicKey);
			notification.TotalFee = Amount(123);

			// Act:
			test::ObserveNotification(observer, notification, context);

			// Assert:
			test::AssertBalances(context.cache(), accountStateIter.get().PublicKey, { { Currency_Mosaic_Id, Amount(987) } });

			// - if signer is remote, it should have an unchanged balance
			if (signerPublicKey != accountStateIter.get().PublicKey)
				test::AssertBalances(context.cache(), signerPublicKey, {});

			// - check (lack of) receipt
			auto pStatement = context.statementBuilder().build();
			ASSERT_EQ(0u, pStatement->TransactionStatements.size());
		});
	}

	// endregion

	// region improper link

	namespace {
		template<typename TMutator>
		void AssertImproperLink(TMutator mutator) {
			// Arrange:
			test::AccountObserverTestContext context(NotifyMode::Commit);
			auto& accountStateCache = context.cache().sub<cache::AccountStateCache>();
			auto pObserver = CreateHarvestFeeObserver(Currency_Mosaic_Id);

			auto signerPublicKey = test::GenerateRandomData<Key_Size>();
			auto accountStateIter = RemoteAccountTraits::AddAccount(accountStateCache, signerPublicKey, Height(1));
			accountStateIter.get().Balances.credit(Currency_Mosaic_Id, Amount(987));
			mutator(accountStateIter.get());

			auto notification = test::CreateBlockNotification(signerPublicKey);
			notification.TotalFee = Amount(123);

			// Act + Assert:
			EXPECT_THROW(test::ObserveNotification(*pObserver, notification, context), catapult_runtime_error);
		}
	}

	TEST(TEST_CLASS, FailureIfLinkedAccountHasWrongType) {
		// Assert:
		AssertImproperLink([](auto& accountState) {
			// Arrange: change the main account to have the wrong type
			accountState.AccountType = state::AccountType::Remote;
		});
	}

	TEST(TEST_CLASS, FailureIfLinkedAccountDoesNotReferenceRemoteAccount) {
		// Assert:
		AssertImproperLink([](auto& accountState) {
			// Arrange: change the main account to point to a different account
			test::FillWithRandomData(accountState.LinkedAccountKey);
		});
	}

	// endregion
}}
