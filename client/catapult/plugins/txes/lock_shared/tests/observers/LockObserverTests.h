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
		using NotificationType = typename TTraits::NotificationType;
		using LockInfoType = typename TTraits::ValueType;

		static constexpr auto Default_Height = Height(888);

	public:
		// region commit

		static void AssertObserverAddsInfoOnCommit() {
			// Arrange:
			auto observerContext = typename TTraits::ObserverTestContext(NotifyMode::Commit, Default_Height);
			auto& lockInfoCacheDelta = observerContext.cache().template sub<typename TTraits::CacheType>();

			typename TTraits::NotificationBuilder notificationBuilder;
			auto notification = notificationBuilder.notification();

			// Act:
			auto pObserver = TTraits::CreateObserver();
			test::ObserveNotification(*pObserver, notification, observerContext);

			// Assert: lock info was added to cache
			EXPECT_EQ(1u, lockInfoCacheDelta.size());

			const auto& key = TTraits::ToKey(notification);
			ASSERT_TRUE(lockInfoCacheDelta.contains(key));

			const auto& lockInfoHistory = lockInfoCacheDelta.find(key).get();
			EXPECT_EQ(key, lockInfoHistory.id());
			ASSERT_EQ(1u, lockInfoHistory.historyDepth());

			AssertLockInfo(notification, lockInfoHistory.back());
			AssertStatement(notification, *observerContext.statementBuilder().build());
		}

		static void AssertObserverCanAddToHistoryWhenInfoIsInactive() {
			// Arrange:
			auto observerContext = typename TTraits::ObserverTestContext(NotifyMode::Commit, Default_Height);
			auto& lockInfoCacheDelta = observerContext.cache().template sub<typename TTraits::CacheType>();

			auto lockInfo = TTraits::CreateLockInfo();
			lockInfo.Status = state::LockStatus::Used;
			lockInfo.EndHeight = Height(1000);
			lockInfoCacheDelta.insert(lockInfo);

			typename TTraits::NotificationBuilder notificationBuilder;
			notificationBuilder.prepare(lockInfo);
			auto notification = notificationBuilder.notification();

			// Act:
			auto pObserver = TTraits::CreateObserver();
			test::ObserveNotification(*pObserver, notification, observerContext);

			// Assert: lock info was added to cache and history size is two
			EXPECT_EQ(1u, lockInfoCacheDelta.size());

			const auto& key = TTraits::ToKey(notification);
			ASSERT_TRUE(lockInfoCacheDelta.contains(key));

			const auto& lockInfoHistory = lockInfoCacheDelta.find(key).get();
			EXPECT_EQ(key, lockInfoHistory.id());
			ASSERT_EQ(2u, lockInfoHistory.historyDepth());

			AssertLockInfo(notification, lockInfoHistory.back());
			AssertStatement(notification, *observerContext.statementBuilder().build());
		}

		// please note:
		// * this should be prevented by validators, so does not need to be prohibited by the observer
		// * this behavior simplifies workaround for SkipSecretLockUniquenessChecks fork
		static void AssertObserverCanAddToHistoryWhenInfoIsActive() {
			// Arrange:
			auto observerContext = typename TTraits::ObserverTestContext(NotifyMode::Commit, Default_Height);
			auto& lockInfoCacheDelta = observerContext.cache().template sub<typename TTraits::CacheType>();

			auto lockInfo = TTraits::CreateLockInfo();
			lockInfo.Status = state::LockStatus::Unused;
			lockInfo.EndHeight = Height(1000);
			lockInfoCacheDelta.insert(lockInfo);

			typename TTraits::NotificationBuilder notificationBuilder;
			notificationBuilder.prepare(lockInfo);
			auto notification = notificationBuilder.notification();

			// Act:
			auto pObserver = TTraits::CreateObserver();
			test::ObserveNotification(*pObserver, notification, observerContext);

			// Assert: lock info was added to cache and history size is two
			EXPECT_EQ(1u, lockInfoCacheDelta.size());

			const auto& key = TTraits::ToKey(notification);
			ASSERT_TRUE(lockInfoCacheDelta.contains(key));

			const auto& lockInfoHistory = lockInfoCacheDelta.find(key).get();
			EXPECT_EQ(key, lockInfoHistory.id());
			ASSERT_EQ(2u, lockInfoHistory.historyDepth());

			AssertLockInfo(notification, lockInfoHistory.back());
			AssertStatement(notification, *observerContext.statementBuilder().build());
		}

	private:
		static void AssertLockInfo(const NotificationType& notification, const LockInfoType& lockInfo) {
			EXPECT_EQ(notification.Owner, lockInfo.OwnerAddress);
			EXPECT_EQ(notification.Mosaic.MosaicId, test::UnresolveXor(lockInfo.MosaicId));
			EXPECT_EQ(notification.Mosaic.Amount, lockInfo.Amount);
			EXPECT_EQ(Default_Height + Height(notification.Duration.unwrap()), lockInfo.EndHeight);

			TTraits::AssertAddedLockInfo(lockInfo, notification);
		}

		static void AssertStatement(const NotificationType& notification, const model::BlockStatement& statement) {
			ASSERT_EQ(1u, statement.TransactionStatements.size());

			const auto& receiptPair = *statement.TransactionStatements.find(model::ReceiptSource());
			ASSERT_EQ(1u, receiptPair.second.size());

			const auto& receipt = static_cast<const model::BalanceChangeReceipt&>(receiptPair.second.receiptAt(0));
			ASSERT_EQ(sizeof(model::BalanceChangeReceipt), receipt.Size);
			EXPECT_EQ(1u, receipt.Version);
			EXPECT_EQ(TTraits::Debit_Receipt_Type, receipt.Type);
			EXPECT_EQ(notification.Mosaic.MosaicId, test::UnresolveXor(receipt.Mosaic.MosaicId));
			EXPECT_EQ(notification.Mosaic.Amount, receipt.Mosaic.Amount);
			EXPECT_EQ(notification.Owner, receipt.TargetAddress);
		}

		// endregion

	public:
		// region rollback

		static void AssertObserverRemovesInfoOnRollbackWhenHistoryDepthIsOne() {
			// Arrange:
			auto observerContext = typename TTraits::ObserverTestContext(NotifyMode::Rollback, Default_Height);
			auto& lockInfoCacheDelta = observerContext.cache().template sub<typename TTraits::CacheType>();

			auto lockInfo = TTraits::CreateLockInfo();
			lockInfo.Status = state::LockStatus::Used;
			lockInfo.EndHeight = Height(1000);
			lockInfoCacheDelta.insert(lockInfo);

			typename TTraits::NotificationBuilder notificationBuilder;
			notificationBuilder.prepare(lockInfo);
			auto notification = notificationBuilder.notification();

			// Act:
			auto pObserver = TTraits::CreateObserver();
			test::ObserveNotification(*pObserver, notification, observerContext);

			// Assert: lock info was removed
			EXPECT_EQ(0u, lockInfoCacheDelta.size());

			auto pStatement = observerContext.statementBuilder().build();
			ASSERT_EQ(0u, pStatement->TransactionStatements.size());
		}

		static void AssertObserverRemovesInfoOnRollbackWhenHistoryDepthIsGreaterThanOne() {
			// Arrange:
			auto observerContext = typename TTraits::ObserverTestContext(NotifyMode::Rollback, Default_Height);
			auto& lockInfoCacheDelta = observerContext.cache().template sub<typename TTraits::CacheType>();

			auto lockInfo = TTraits::CreateLockInfo();
			lockInfo.Status = state::LockStatus::Used;
			lockInfo.EndHeight = Height(1000);
			lockInfoCacheDelta.insert(lockInfo);

			auto lockInfo2 = lockInfo;
			lockInfo2.EndHeight = Height(900);
			lockInfoCacheDelta.insert(lockInfo2);

			typename TTraits::NotificationBuilder notificationBuilder;
			notificationBuilder.prepare(lockInfo);
			auto notification = notificationBuilder.notification();

			// Act:
			auto pObserver = TTraits::CreateObserver();
			test::ObserveNotification(*pObserver, notification, observerContext);

			// Assert: lock info was removed
			EXPECT_EQ(1u, lockInfoCacheDelta.size());

			const auto& key = TTraits::ToKey(notification);
			ASSERT_TRUE(lockInfoCacheDelta.contains(key));

			const auto& lockInfoHistory = lockInfoCacheDelta.find(key).get();
			EXPECT_EQ(key, lockInfoHistory.id());
			ASSERT_EQ(1u, lockInfoHistory.historyDepth());

			TTraits::AssertEqual(lockInfo, lockInfoHistory.back());

			auto pStatement = observerContext.statementBuilder().build();
			ASSERT_EQ(0u, pStatement->TransactionStatements.size());
		}
	};

	// endregion
}}

#define MAKE_LOCK_OBSERVER_TEST(TRAITS_NAME, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { LockObserverTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_LOCK_OBSERVER_TESTS(TRAITS_NAME) \
	MAKE_LOCK_OBSERVER_TEST(TRAITS_NAME, ObserverAddsInfoOnCommit) \
	MAKE_LOCK_OBSERVER_TEST(TRAITS_NAME, ObserverCanAddToHistoryWhenInfoIsInactive) \
	MAKE_LOCK_OBSERVER_TEST(TRAITS_NAME, ObserverCanAddToHistoryWhenInfoIsActive) \
	\
	MAKE_LOCK_OBSERVER_TEST(TRAITS_NAME, ObserverRemovesInfoOnRollbackWhenHistoryDepthIsOne) \
	MAKE_LOCK_OBSERVER_TEST(TRAITS_NAME, ObserverRemovesInfoOnRollbackWhenHistoryDepthIsGreaterThanOne)
