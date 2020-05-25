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
			auto lockInfo = TTraits::BasicTraits::CreateLockInfo();

			// Act:
			RunTest(
					typename TTraits::ObserverTestContext(NotifyMode::Commit),
					lockInfo,
					[&lockInfo, initialAmount](auto& cache, auto& accountState) {
						cache.insert(lockInfo);
						if (Amount(0) != initialAmount)
							accountState.Balances.credit(lockInfo.MosaicId, initialAmount);
					},
					[&lockInfo, initialAmount](const auto& lockInfoCache, const auto& accountState, auto& observerContext) {
						// Assert: status and balance
						const auto& key = TTraits::BasicTraits::ToKey(lockInfo);
						const auto& result = lockInfoCache.find(key).get();
						EXPECT_EQ(state::LockStatus::Used, result.Status);
						auto expectedBalance = lockInfo.Amount + initialAmount;
						EXPECT_EQ(expectedBalance, accountState.Balances.get(result.MosaicId));

						auto pStatement = observerContext.statementBuilder().build();
						ASSERT_EQ(1u, pStatement->TransactionStatements.size());
						const auto& receiptPair = *pStatement->TransactionStatements.find(model::ReceiptSource());
						ASSERT_EQ(1u, receiptPair.second.size());

						const auto& receipt = static_cast<const model::BalanceChangeReceipt&>(receiptPair.second.receiptAt(0));
						ASSERT_EQ(sizeof(model::BalanceChangeReceipt), receipt.Size);
						EXPECT_EQ(1u, receipt.Version);
						EXPECT_EQ(TTraits::Receipt_Type, receipt.Type);
						EXPECT_EQ(lockInfo.MosaicId, receipt.Mosaic.MosaicId);
						EXPECT_EQ(lockInfo.Amount, receipt.Mosaic.Amount);
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
			auto lockInfo = TTraits::BasicTraits::CreateLockInfo();
			lockInfo.Status = state::LockStatus::Used;

			// Act:
			RunTest(
					typename TTraits::ObserverTestContext(NotifyMode::Rollback),
					lockInfo,
					[&lockInfo](auto& cache, auto& accountState) {
						cache.insert(lockInfo);
						accountState.Balances.credit(lockInfo.MosaicId, lockInfo.Amount + Amount(100));
					},
					[&lockInfo](const auto& lockInfoCache, const auto& accountState, auto& observerContext) {
						// Assert: status and balance
						const auto& key = TTraits::BasicTraits::ToKey(lockInfo);
						const auto& result = lockInfoCache.find(key).get();
						EXPECT_EQ(state::LockStatus::Unused, result.Status);
						EXPECT_EQ(Amount(100), accountState.Balances.get(lockInfo.MosaicId));

						auto pStatement = observerContext.statementBuilder().build();
						ASSERT_EQ(0u, pStatement->TransactionStatements.size());
					});
		}

	private:
		template<typename TSeedCacheFunc, typename TCheckCacheFunc>
		static void RunTest(
				typename TTraits::ObserverTestContext&& context,
				const typename TTraits::BasicTraits::ValueType& lockInfo,
				TSeedCacheFunc seedCache,
				TCheckCacheFunc checkCache) {
			// Arrange:
			auto& accountStateCacheDelta = context.cache().template sub<cache::AccountStateCache>();
			auto accountIdentifier = TTraits::DestinationAccount(lockInfo);
			accountStateCacheDelta.addAccount(accountIdentifier, Height(1));
			auto& accountState = accountStateCacheDelta.find(accountIdentifier).get();

			auto& lockInfoCacheDelta = context.cache().template sub<typename TTraits::BasicTraits::CacheType>();
			seedCache(lockInfoCacheDelta, accountState);

			auto pObserver = TTraits::CreateObserver();

			// Act:
			typename TTraits::NotificationBuilder notificationBuilder;
			notificationBuilder.prepare(lockInfo);

			auto notification = notificationBuilder.notification();
			test::ObserveNotification(*pObserver, notification, context);

			// Assert
			checkCache(lockInfoCacheDelta, accountState, context);
		}
	};
}}

#define MAKE_LOCK_STATUS_OBSERVER_TEST(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { LockStatusObserverTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_LOCK_STATUS_OBSERVER_TESTS(TRAITS_NAME) \
	MAKE_LOCK_STATUS_OBSERVER_TEST(TRAITS_NAME, ObserverSetsStatusToUsedAndCreditsBalanceOnCommit) \
	MAKE_LOCK_STATUS_OBSERVER_TEST(TRAITS_NAME, ObserverSetsStatusToUsedAndCreditsToExistingBalanceOnCommit) \
	MAKE_LOCK_STATUS_OBSERVER_TEST(TRAITS_NAME, ObserverSetsStatusToUnusedAndDebitsBalanceOnRollback)
