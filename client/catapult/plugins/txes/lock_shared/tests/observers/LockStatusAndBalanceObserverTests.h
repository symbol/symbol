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

#pragma once
#include "catapult/cache_core/AccountStateCache.h"
#include "plugins/txes/lock_shared/tests/test/LockInfoCacheTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"

namespace catapult { namespace observers {

	/// Test suite for observers using LockStatusAccountBalanceObserver helper.
	template<typename TTraits>
	struct LockStatusObserverTests {
	private:
		static void AssertObserverSetsStatusToUsedAndCreditsBalanceOnCommit(Amount initialAmount) {
			// Arrange:
			auto observerContext = typename TTraits::ObserverTestContext(NotifyMode::Commit);

			// Act:
			RunTest(std::move(observerContext), initialAmount, Amount(100), [&](
					const auto& accountState,
					const auto& originalLockInfo,
					const auto& lockInfoHistory,
					const auto& statement) {
				// Assert: status and balance
				EXPECT_EQ(GetLockIdentifier(originalLockInfo), lockInfoHistory.id());
				ASSERT_EQ(1u, lockInfoHistory.historyDepth());

				const auto& lockInfo = lockInfoHistory.back();
				EXPECT_EQ(state::LockStatus::Used, lockInfo.Status);
				EXPECT_EQ(initialAmount + Amount(100), accountState.Balances.get(lockInfo.MosaicId));

				// - check receipt
				ASSERT_EQ(1u, statement.TransactionStatements.size());

				const auto& receiptPair = *statement.TransactionStatements.find(model::ReceiptSource());
				ASSERT_EQ(1u, receiptPair.second.size());

				const auto& receipt = static_cast<const model::BalanceChangeReceipt&>(receiptPair.second.receiptAt(0));
				ASSERT_EQ(sizeof(model::BalanceChangeReceipt), receipt.Size);
				EXPECT_EQ(1u, receipt.Version);
				EXPECT_EQ(TTraits::Receipt_Type, receipt.Type);
				EXPECT_EQ(originalLockInfo.MosaicId, receipt.Mosaic.MosaicId);
				EXPECT_EQ(originalLockInfo.Amount, receipt.Mosaic.Amount);
				EXPECT_EQ(accountState.Address, receipt.TargetAddress);
			});
		}

	public:
		static void AssertObserverSetsStatusToUsedAndCreditsBalanceOnCommit() {
			AssertObserverSetsStatusToUsedAndCreditsBalanceOnCommit(Amount(0));
		}

		static void AssertObserverSetsStatusToUsedAndCreditsToExistingBalanceOnCommit() {
			AssertObserverSetsStatusToUsedAndCreditsBalanceOnCommit(Amount(100));
		}

		static void AssertObserverSetsStatusToUnusedAndDebitsBalanceOnRollback() {
			// Arrange:
			auto observerContext = typename TTraits::ObserverTestContext(NotifyMode::Rollback);

			// Act:
			RunTest(std::move(observerContext), Amount(1000), Amount(100), [&](
					const auto& accountState,
					const auto& originalLockInfo,
					const auto& lockInfoHistory,
					const auto& statement) {
				// Assert: status and balance
				EXPECT_EQ(GetLockIdentifier(originalLockInfo), lockInfoHistory.id());
				ASSERT_EQ(1u, lockInfoHistory.historyDepth());

				const auto& lockInfo = lockInfoHistory.back();
				EXPECT_EQ(state::LockStatus::Unused, lockInfo.Status);
				EXPECT_EQ(Amount(900), accountState.Balances.get(originalLockInfo.MosaicId));

				ASSERT_EQ(0u, statement.TransactionStatements.size());
			});
		}

	private:
		template<typename TCheckCacheFunc>
		static void RunTest(
				typename TTraits::ObserverTestContext&& observerContext,
				Amount initialAmount,
				Amount lockAmount,
				TCheckCacheFunc checkCache) {
			// Arrange:
			auto pObserver = TTraits::CreateObserver();

			auto originalLockInfo = TTraits::BasicTraits::CreateLockInfo();
			originalLockInfo.Amount = lockAmount;
			originalLockInfo.Status = static_cast<state::LockStatus>(77); // status should be updated by observer

			auto& accountStateCacheDelta = observerContext.cache().template sub<cache::AccountStateCache>();
			auto accountIdentifier = TTraits::DestinationAccount(originalLockInfo);
			accountStateCacheDelta.addAccount(accountIdentifier, Height(1));
			auto& accountState = accountStateCacheDelta.find(accountIdentifier).get();

			auto& lockInfoCacheDelta = observerContext.cache().template sub<typename TTraits::BasicTraits::CacheType>();
			lockInfoCacheDelta.insert(originalLockInfo);
			if (Amount(0) != initialAmount)
				accountState.Balances.credit(originalLockInfo.MosaicId, initialAmount);

			// Act:
			typename TTraits::NotificationBuilder notificationBuilder;
			notificationBuilder.prepare(originalLockInfo);

			auto notification = notificationBuilder.notification();
			test::ObserveNotification(*pObserver, notification, observerContext);

			// Assert
			const auto& lockInfoHistory = lockInfoCacheDelta.find(GetLockIdentifier(originalLockInfo)).get();
			checkCache(accountState, originalLockInfo, lockInfoHistory, *observerContext.statementBuilder().build());
		}
	};
}}

#define MAKE_LOCK_STATUS_OBSERVER_TEST(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { LockStatusObserverTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_LOCK_STATUS_OBSERVER_TESTS(TRAITS_NAME) \
	MAKE_LOCK_STATUS_OBSERVER_TEST(TRAITS_NAME, ObserverSetsStatusToUsedAndCreditsBalanceOnCommit) \
	MAKE_LOCK_STATUS_OBSERVER_TEST(TRAITS_NAME, ObserverSetsStatusToUsedAndCreditsToExistingBalanceOnCommit) \
	MAKE_LOCK_STATUS_OBSERVER_TEST(TRAITS_NAME, ObserverSetsStatusToUnusedAndDebitsBalanceOnRollback)
