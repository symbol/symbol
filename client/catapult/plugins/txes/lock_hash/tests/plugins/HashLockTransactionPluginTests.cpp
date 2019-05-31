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

#include "src/plugins/HashLockTransactionPlugin.h"
#include "src/model/HashLockNotifications.h"
#include "src/model/HashLockTransaction.h"
#include "plugins/txes/lock_shared/tests/test/LockTransactionUtils.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS HashLockTransactionPluginTests

	// region TransactionPlugin

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(HashLock, 1, 1,)
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , Entity_Type_Hash_Lock)

	PLUGIN_TEST(CanCalculateSize) {
		// Arrange:
		auto pPlugin = TTraits::CreatePlugin();
		typename TTraits::TransactionType transaction;

		// Act:
		auto realSize = pPlugin->calculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(sizeof(typename TTraits::TransactionType), realSize);
	}

	// endregion

	// region accounts extraction

	PLUGIN_TEST(CanExtractAccounts) {
		// Arrange:
		mocks::MockNotificationSubscriber sub;
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction;

		// Act:
		test::PublishTransaction(*pPlugin, transaction, sub);

		// Assert:
		EXPECT_EQ(4u, sub.numNotifications());
		EXPECT_EQ(0u, sub.numAddresses());
		EXPECT_EQ(0u, sub.numKeys());
	}

	// endregion

	// region duration notification

	PLUGIN_TEST(CanPublishDurationNotification) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<HashLockDurationNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();
		auto pTransaction = test::CreateRandomLockTransaction<TTraits>();
		pTransaction->Duration = test::GenerateRandomValue<BlockDuration>();

		// Act:
		test::PublishTransaction(*pPlugin, *pTransaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		EXPECT_EQ(pTransaction->Duration, notification.Duration);
	}

	// endregion

	// region mosaic notification

	PLUGIN_TEST(CanPublishMosaicNotification) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<HashLockMosaicNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();
		auto pTransaction = test::CreateRandomLockTransaction<TTraits>();
		pTransaction->Mosaic = { test::GenerateRandomValue<UnresolvedMosaicId>(), test::GenerateRandomValue<Amount>() };

		// Act:
		test::PublishTransaction(*pPlugin, *pTransaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		EXPECT_EQ(pTransaction->Mosaic.MosaicId, notification.Mosaic.MosaicId);
		EXPECT_EQ(pTransaction->Mosaic.Amount, notification.Mosaic.Amount);
	}

	// endregion

	// region transaction hash notification

	namespace {
		template<typename TTransaction>
		void AssertHashLockNotification(const HashLockNotification& notification, const TTransaction& transaction) {
			test::AssertBaseLockNotification(notification, transaction);
			EXPECT_EQ(transaction.Hash, notification.Hash);
		}
	}

	PLUGIN_TEST(CanPublishTransactionHashNotification) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<HashLockNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();
		auto pTransaction = test::CreateRandomLockTransaction<TTraits>();

		// Act:
		test::PublishTransaction(*pPlugin, *pTransaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		AssertHashLockNotification(notification, *pTransaction);
	}

	// endregion

	// region balance transfer

	PLUGIN_TEST(CanPublishBalanceDebitNotification) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<model::BalanceDebitNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();
		auto pTransaction = test::CreateRandomLockTransaction<TTraits>();

		// Act:
		test::PublishTransaction(*pPlugin, *pTransaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		EXPECT_EQ(pTransaction->Signer, notification.Sender);
		EXPECT_EQ(pTransaction->Mosaic.MosaicId, notification.MosaicId);
		EXPECT_EQ(pTransaction->Mosaic.Amount, notification.Amount);
	}

	// endregion
}}
