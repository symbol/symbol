#pragma once
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/observers/ObserverContext.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Lock observer test suite.
	template<typename TTraits>
	struct LockObserverTests {
	private:
		static auto DefaultHeight() {
			return Height(888);
		}

	public:
		// region commit

		static void AssertObserverAddsInfoOnCommit() {
			// Act:
			RunTest(
					typename TTraits::ObserverTestContext(observers::NotifyMode::Commit, DefaultHeight()),
					[](const auto&, auto& ownerState, const auto& notification) {
						ownerState.Balances.credit(notification.Mosaic.MosaicId, notification.Mosaic.Amount);
					},
					// Assert: lock info was added to cache
					[](const auto& lockInfoCacheDelta, const auto& ownerState, const auto& notification) {
						EXPECT_EQ(1u, lockInfoCacheDelta.size());

						const auto& key = TTraits::ToKey(notification);
						ASSERT_TRUE(lockInfoCacheDelta.contains(key));

						const auto& lockInfo = lockInfoCacheDelta.get(key);
						EXPECT_EQ(ownerState.PublicKey, lockInfo.Account);
						EXPECT_EQ(notification.Mosaic.MosaicId, lockInfo.MosaicId);
						EXPECT_EQ(notification.Mosaic.Amount, lockInfo.Amount);
						EXPECT_EQ(DefaultHeight() + Height(notification.Duration.unwrap()), lockInfo.Height);

						TTraits::AssertAddedLockInfo(lockInfo, notification);
					});
		}

		static void AssertObserverDoesNotOverwriteInfoOnCommit() {
			// Arrange:
			typename TTraits::ObserverTestContext context(observers::NotifyMode::Commit, DefaultHeight());
			typename TTraits::NotificationBuilder notificationBuilder;
			auto notification = notificationBuilder.notification();

			auto pObserver = TTraits::CreateObserver();
			auto& accountStateCacheDelta = context.cache().template sub<cache::AccountStateCache>();
			auto& ownerState = accountStateCacheDelta.addAccount(notification.Signer, Height(1));
			ownerState.Balances.credit(notification.Mosaic.MosaicId, notification.Mosaic.Amount);

			// - seed with lock info with same hash
			auto& lockInfoCacheDelta = context.cache().template sub<typename TTraits::CacheType>();
			auto lockInfo = TTraits::GenerateRandomLockInfo(notification);
			lockInfoCacheDelta.insert(lockInfo);

			// Act + Assert:
			EXPECT_THROW(test::ObserveNotification(*pObserver, notification, context), catapult_invalid_argument);
		}

		static void AssertObserverDebitsSignerBalanceOnCommit() {
			// Act:
			RunTest(
					typename TTraits::ObserverTestContext(observers::NotifyMode::Commit, DefaultHeight()),
					[](const auto&, auto& ownerState, const auto& notification) {
						ownerState.Balances.credit(notification.Mosaic.MosaicId, notification.Mosaic.Amount + Amount(100));
					},
					[](const auto&, const auto& ownerState, const auto& notification) {
						// Assert: owner balance has been debited
						EXPECT_EQ(Amount(100), ownerState.Balances.get(notification.Mosaic.MosaicId));
					});
		}

		// endregion

		// region rollback

		static void AssertObserverRemovesInfoOnRollback() {
			// Act:
			RunTest(
					typename TTraits::ObserverTestContext(observers::NotifyMode::Rollback, DefaultHeight()),
					[](auto& lockInfoCacheDelta, const auto&, const auto& notification) {
						auto lockInfo = TTraits::GenerateRandomLockInfo(notification);
						lockInfoCacheDelta.insert(lockInfo);
					},
					[](const auto& lockInfoCacheDelta, const auto&, const auto&) {
						// Assert: lock info was removed
						EXPECT_EQ(0u, lockInfoCacheDelta.size());
					});
		}

		static void AssertObserverCreditsSignerBalanceOnRollback() {
			// Act:
			RunTest(
					typename TTraits::ObserverTestContext(observers::NotifyMode::Rollback, DefaultHeight()),
					[](auto& lockInfoCacheDelta, auto& ownerState, const auto& notification) {
						auto lockInfo = TTraits::GenerateRandomLockInfo(notification);
						lockInfoCacheDelta.insert(lockInfo);
						ownerState.Balances.credit(notification.Mosaic.MosaicId, Amount(100));
					},
					[](const auto&, const auto& ownerState, const auto& notification) {
						// Assert: owner balance has been credited
						EXPECT_EQ(Amount(100) + notification.Mosaic.Amount, ownerState.Balances.get(notification.Mosaic.MosaicId));
					});
		}

		// endregion

	private:
		template<typename TSeedCacheFunc, typename TCheckCacheFunc>
		static void RunTest(typename TTraits::ObserverTestContext&& context, TSeedCacheFunc seedCache, TCheckCacheFunc checkCache) {
			// Arrange:
			typename TTraits::NotificationBuilder notificationBuilder;
			auto notification = notificationBuilder.notification();

			auto pObserver = TTraits::CreateObserver();
			auto& accountStateCacheDelta = context.cache().template sub<cache::AccountStateCache>();
			auto& ownerState = accountStateCacheDelta.addAccount(notification.Signer, Height(1));

			auto& lockInfoCacheDelta = context.cache().template sub<typename TTraits::CacheType>();
			seedCache(lockInfoCacheDelta, ownerState, notification);

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert: check the cache
			checkCache(lockInfoCacheDelta, ownerState, notification);
		}
	};
}}

#define MAKE_LOCK_OBSERVER_TEST(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::LockObserverTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_LOCK_OBSERVER_TESTS(TRAITS_NAME) \
	MAKE_LOCK_OBSERVER_TEST(TRAITS_NAME, ObserverAddsInfoOnCommit) \
	MAKE_LOCK_OBSERVER_TEST(TRAITS_NAME, ObserverDoesNotOverwriteInfoOnCommit) \
	MAKE_LOCK_OBSERVER_TEST(TRAITS_NAME, ObserverRemovesInfoOnRollback) \
	MAKE_LOCK_OBSERVER_TEST(TRAITS_NAME, ObserverDebitsSignerBalanceOnCommit) \
	MAKE_LOCK_OBSERVER_TEST(TRAITS_NAME, ObserverCreditsSignerBalanceOnRollback)
