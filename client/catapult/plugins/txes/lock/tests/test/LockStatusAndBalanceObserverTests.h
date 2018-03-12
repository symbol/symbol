#pragma once
#include "LockInfoCacheTestUtils.h"
#include "LockNotificationsTestUtils.h"
#include "src/observers/Observers.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "tests/test/plugins/ObserverTestUtils.h"

namespace catapult { namespace test {

	/// Test suite for observers using LockStatusAccountBalanceObserver helper.
	template<typename TTraits>
	struct LockStatusObserverTests {
	private:
		static void AssertObserverSetsStatusToUsedAndCreditsBalanceOnCommit(Amount initialAmount) {
			// Arrange:
			auto lockInfo = TTraits::BasicTraits::CreateLockInfo();

			// Act:
			RunTest(
					typename TTraits::ObserverTestContext(observers::NotifyMode::Commit),
					lockInfo,
					[&lockInfo, initialAmount](auto& cache, auto& accountState) {
						cache.insert(lockInfo);
						if (Amount(0) != initialAmount)
							accountState.Balances.credit(lockInfo.MosaicId, initialAmount);
					},
					[&lockInfo, initialAmount](const auto& lockInfoCache, const auto& accountState) {
						// Assert: status and balance
						const auto& key = TTraits::BasicTraits::ToKey(lockInfo);
						const auto& result = lockInfoCache.get(key);
						EXPECT_EQ(model::LockStatus::Used, result.Status);
						auto expectedBalance = lockInfo.Amount + initialAmount;
						EXPECT_EQ(expectedBalance, accountState.Balances.get(result.MosaicId));
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
			lockInfo.Status = model::LockStatus::Used;

			// Act:
			RunTest(
					typename TTraits::ObserverTestContext(observers::NotifyMode::Rollback),
					lockInfo,
					[&lockInfo](auto& cache, auto& accountState) {
						cache.insert(lockInfo);
						accountState.Balances.credit(lockInfo.MosaicId, lockInfo.Amount + Amount(100));
					},
					[&lockInfo](const auto& lockInfoCache, const auto& accountState) {
						// Assert: status and balance
						const auto& key = TTraits::BasicTraits::ToKey(lockInfo);
						const auto& result = lockInfoCache.get(key);
						EXPECT_EQ(model::LockStatus::Unused, result.Status);
						EXPECT_EQ(Amount(100), accountState.Balances.get(lockInfo.MosaicId));
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
			auto& accountState = accountStateCacheDelta.addAccount(TTraits::DestinationAccount(lockInfo), Height(1));

			auto& lockInfoCacheDelta = context.cache().template sub<typename TTraits::BasicTraits::CacheType>();
			seedCache(lockInfoCacheDelta, accountState);

			auto pObserver = TTraits::CreateObserver();

			// Act:
			typename TTraits::NotificationBuilder notificationBuilder;
			const auto& key = TTraits::BasicTraits::ToKey(lockInfo);
			notificationBuilder.setHash(key);

			auto notification = notificationBuilder.notification();
			test::ObserveNotification(*pObserver, notification, context);

			// Assert
			checkCache(lockInfoCacheDelta, accountState);
		}
	};
}}

#define MAKE_LOCK_STATUS_OBSERVER_TEST(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::LockStatusObserverTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_LOCK_STATUS_OBSERVER_TESTS(TRAITS_NAME) \
	MAKE_LOCK_STATUS_OBSERVER_TEST(TRAITS_NAME, ObserverSetsStatusToUsedAndCreditsBalanceOnCommit) \
	MAKE_LOCK_STATUS_OBSERVER_TEST(TRAITS_NAME, ObserverSetsStatusToUsedAndCreditsToExistingBalanceOnCommit) \
	MAKE_LOCK_STATUS_OBSERVER_TEST(TRAITS_NAME, ObserverSetsStatusToUnusedAndDebitsBalanceOnRollback)
