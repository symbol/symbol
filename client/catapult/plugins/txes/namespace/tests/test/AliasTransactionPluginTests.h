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
#include "src/model/AliasNotifications.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"

namespace catapult { namespace test {

	/// Alias transaction plugin test suite.
	template<typename TTraits>
	class AliasTransactionPluginTests {
	public:
		static void AssertCanCalculateSize() {
			// Arrange:
			auto pPlugin = TTraits::CreatePlugin();

			typename TTraits::TransactionType transaction;
			transaction.Size = 0;

			// Act:
			auto realSize = pPlugin->calculateRealSize(transaction);

			// Assert:
			EXPECT_EQ(sizeof(typename TTraits::TransactionType), realSize);
		}

		template<typename TNotificationTraits>
		static void AssertCanPublishCorrectNumberOfNotifications() {
			// Arrange:
			mocks::MockNotificationSubscriber sub;
			auto pPlugin = TTraits::CreatePlugin();

			typename TTraits::TransactionType transaction;

			// Act:
			test::PublishTransaction(*pPlugin, transaction, sub);

			// Assert:
			EXPECT_EQ(TNotificationTraits::NumNotifications(), sub.numNotifications());
		}

		template<typename TNotificationTraits>
		static void AssertCanExtractAliasNotifications() {
			// Arrange:
			mocks::MockTypedNotificationSubscriber<model::AliasOwnerNotification> aliasOwnerSub;
			mocks::MockTypedNotificationSubscriber<typename TNotificationTraits::Notification_Type> aliasedSub;
			auto pPlugin = TTraits::CreatePlugin();

			typename TTraits::TransactionType transaction;
			transaction.NamespaceId = NamespaceId(123);
			transaction.AliasAction = model::AliasAction::Unlink;
			transaction.Signer = test::GenerateRandomByteArray<Key>();
			test::FillWithRandomData(TNotificationTraits::TransactionAlias(transaction));

			// Act:
			test::PublishTransaction(*pPlugin, transaction, aliasOwnerSub);
			test::PublishTransaction(*pPlugin, transaction, aliasedSub);

			// Assert:
			ASSERT_EQ(1u, aliasOwnerSub.numMatchingNotifications());
			const auto& aliasOwnerNotification = aliasOwnerSub.matchingNotifications()[0];
			EXPECT_EQ(NamespaceId(123), aliasOwnerNotification.NamespaceId);
			EXPECT_EQ(model::AliasAction::Unlink, aliasOwnerNotification.AliasAction);
			EXPECT_EQ(transaction.Signer, aliasOwnerNotification.Owner);

			ASSERT_EQ(1u, aliasedSub.numMatchingNotifications());
			const auto& aliasedDataNotification = aliasedSub.matchingNotifications()[0];
			EXPECT_EQ(NamespaceId(123), aliasedDataNotification.NamespaceId);
			EXPECT_EQ(model::AliasAction::Unlink, aliasedDataNotification.AliasAction);
			EXPECT_EQ(TNotificationTraits::TransactionAlias(transaction), aliasedDataNotification.AliasedData);
		}
	};

#define DEFINE_ALIAS_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, TRAITS_PREFIX, NOTIFICATION_TRAITS) \
	PLUGIN_TEST(CanCalculateSize) { \
		test::AliasTransactionPluginTests<TTraits>::AssertCanCalculateSize(); \
	} \
	PLUGIN_TEST(CanPublishCorrectNumberOfNotifications) { \
		test::AliasTransactionPluginTests<TTraits>::template AssertCanPublishCorrectNumberOfNotifications<NOTIFICATION_TRAITS>(); \
	} \
	PLUGIN_TEST(CanExtractAliasNotifications) { \
		test::AliasTransactionPluginTests<TTraits>::template AssertCanExtractAliasNotifications<NOTIFICATION_TRAITS>(); \
	}
}}
