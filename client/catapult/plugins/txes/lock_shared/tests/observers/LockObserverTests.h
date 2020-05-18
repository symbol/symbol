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
#include "sdk/src/extensions/ConversionExtensions.h"
#include "catapult/observers/ObserverContext.h"
#include "plugins/txes/lock_shared/tests/test/LockInfoCacheTestUtils.h"
#include "tests/test/core/ResolverTestUtils.h"
#include "tests/test/plugins/ObserverTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace observers {

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
					typename TTraits::ObserverTestContext(NotifyMode::Commit, DefaultHeight()),
					[](const auto&, const auto&) {
					},
					// Assert: lock info was added to cache
					[](const auto& lockInfoCacheDelta, const auto& notification, auto& observerContext) {
						EXPECT_EQ(1u, lockInfoCacheDelta.size());

						const auto& key = TTraits::ToKey(notification);
						ASSERT_TRUE(lockInfoCacheDelta.contains(key));

						const auto& lockInfo = lockInfoCacheDelta.find(key).get();
						EXPECT_EQ(notification.Owner, lockInfo.SenderPublicKey);
						EXPECT_EQ(notification.Mosaic.MosaicId, test::UnresolveXor(lockInfo.MosaicId));
						EXPECT_EQ(notification.Mosaic.Amount, lockInfo.Amount);
						EXPECT_EQ(DefaultHeight() + Height(notification.Duration.unwrap()), lockInfo.EndHeight);

						TTraits::AssertAddedLockInfo(lockInfo, notification);

						auto pStatement = observerContext.statementBuilder().build();
						ASSERT_EQ(1u, pStatement->TransactionStatements.size());
						const auto& receiptPair = *pStatement->TransactionStatements.find(model::ReceiptSource());
						ASSERT_EQ(1u, receiptPair.second.size());

						const auto& receipt = static_cast<const model::BalanceChangeReceipt&>(receiptPair.second.receiptAt(0));
						ASSERT_EQ(sizeof(model::BalanceChangeReceipt), receipt.Size);
						EXPECT_EQ(1u, receipt.Version);
						EXPECT_EQ(TTraits::Debit_Receipt_Type, receipt.Type);
						EXPECT_EQ(notification.Mosaic.MosaicId, test::UnresolveXor(receipt.Mosaic.MosaicId));
						EXPECT_EQ(notification.Mosaic.Amount, receipt.Mosaic.Amount);
						EXPECT_EQ(notification.Owner, receipt.TargetPublicKey);
					});
		}

		static void AssertObserverDoesNotOverwriteInfoOnCommit() {
			// Arrange:
			typename TTraits::ObserverTestContext context(NotifyMode::Commit, DefaultHeight());
			typename TTraits::NotificationBuilder notificationBuilder;
			auto notification = notificationBuilder.notification();

			auto pObserver = TTraits::CreateObserver();

			// - seed with lock info with same hash
			auto& lockInfoCacheDelta = context.cache().template sub<typename TTraits::CacheType>();
			auto lockInfo = TTraits::GenerateRandomLockInfo(notification);
			lockInfoCacheDelta.insert(lockInfo);

			// Act + Assert:
			EXPECT_THROW(test::ObserveNotification(*pObserver, notification, context), catapult_invalid_argument);
		}

		// endregion

		// region rollback

		static void AssertObserverRemovesInfoOnRollback() {
			// Act:
			RunTest(
					typename TTraits::ObserverTestContext(NotifyMode::Rollback, DefaultHeight()),
					[](auto& lockInfoCacheDelta, const auto& notification) {
						auto lockInfo = TTraits::GenerateRandomLockInfo(notification);
						lockInfoCacheDelta.insert(lockInfo);
					},
					[](const auto& lockInfoCacheDelta, const auto&, auto& observerContext) {
						// Assert: lock info was removed
						EXPECT_EQ(0u, lockInfoCacheDelta.size());

						auto pStatement = observerContext.statementBuilder().build();
						ASSERT_EQ(0u, pStatement->TransactionStatements.size());
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

			auto& lockInfoCacheDelta = context.cache().template sub<typename TTraits::CacheType>();
			seedCache(lockInfoCacheDelta, notification);

			// Act:
			test::ObserveNotification(*pObserver, notification, context);

			// Assert: check the cache
			checkCache(lockInfoCacheDelta, notification, context);
		}
	};
}}

#define MAKE_LOCK_OBSERVER_TEST(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { LockObserverTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_LOCK_OBSERVER_TESTS(TRAITS_NAME) \
	MAKE_LOCK_OBSERVER_TEST(TRAITS_NAME, ObserverAddsInfoOnCommit) \
	MAKE_LOCK_OBSERVER_TEST(TRAITS_NAME, ObserverDoesNotOverwriteInfoOnCommit) \
	MAKE_LOCK_OBSERVER_TEST(TRAITS_NAME, ObserverRemovesInfoOnRollback)
