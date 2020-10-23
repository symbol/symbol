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

#include "catapult/model/TransactionPluginFactory.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS TransactionPluginFactoryTests

	namespace {
		constexpr auto Mock_Transaction_Type = static_cast<EntityType>(0x4FFF);

		template<typename TTransaction>
		void Publish(const TTransaction& transaction, const PublishContext& context, NotificationSubscriber& sub) {
			// raise notifications dependent on the transaction and context data
			sub.notify(AccountPublicKeyNotification(transaction.SignerPublicKey));
			sub.notify(mocks::MockAddressNotification(context.SignerAddress));
		}

		template<TransactionPluginFactoryOptions Options>
		struct RegularTraits {
			using TransactionType = mocks::MockTransaction;
			static constexpr auto Min_Supported_Version = TransactionType::Current_Version;
			static constexpr auto Max_Supported_Version = TransactionType::Current_Version;

			static void SetVersion(const TransactionType&)
			{}

			static auto CreatePlugin() {
				return TransactionPluginFactory<Options>::template Create<mocks::MockTransaction, mocks::EmbeddedMockTransaction>(
						Publish<mocks::MockTransaction>,
						Publish<mocks::EmbeddedMockTransaction>);
			}
		};

		using DefaultRegularTraits = RegularTraits<TransactionPluginFactoryOptions::Default>;
		using OnlyEmbeddableRegularTraits = RegularTraits<TransactionPluginFactoryOptions::Only_Embeddable>;

		template<TransactionPluginFactoryOptions Options>
		struct EmbeddedTraits {
			using TransactionType = mocks::EmbeddedMockTransaction;
			static constexpr auto Min_Supported_Version = TransactionType::Current_Version;
			static constexpr auto Max_Supported_Version = TransactionType::Current_Version;

			static void SetVersion(const TransactionType&)
			{}

			static auto CreatePlugin() {
				return TransactionPluginFactory<Options>::template CreateEmbedded<mocks::EmbeddedMockTransaction>(
						Publish<mocks::EmbeddedMockTransaction>);
			}
		};

		using DefaultEmbeddedTraits = EmbeddedTraits<TransactionPluginFactoryOptions::Default>;
		using OnlyEmbeddableEmbeddedTraits = EmbeddedTraits<TransactionPluginFactoryOptions::Only_Embeddable>;
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, Default, _Default, Mock_Transaction_Type)
	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS_ONLY_EMBEDDABLE(TEST_CLASS, OnlyEmbeddable, _OnlyEmbeddable, Mock_Transaction_Type)

	namespace {
		template<typename TTraits, typename TSubscriber, typename TCheckPublish>
		void RunPublishTest(TCheckPublish checkPublish) {
			// Arrange:
			auto pPlugin = TTraits::CreatePlugin();

			typename TTraits::TransactionType transaction;
			test::FillWithRandomData(transaction);

			PublishContext context;
			test::FillWithRandomData(context.SignerAddress);

			TSubscriber sub;

			// Act:
			pPlugin->publish(transaction, context, sub);

			// Assert:
			ASSERT_EQ(2u, sub.numNotifications());
			checkPublish(transaction, context, sub);
		}
	}

	PLUGIN_TEST_WITH_PREFIXED_TRAITS(CanPublishAllNotifications, Default, _Default) {
		// Act:
		using Subscriber = mocks::MockNotificationSubscriber;
		RunPublishTest<TTraits, Subscriber>([](const auto&, const auto&, const auto& sub) {
			// Assert:
			EXPECT_EQ(AccountPublicKeyNotification::Notification_Type, sub.notificationTypes()[0]);
			EXPECT_EQ(mocks::MockAddressNotification::Notification_Type, sub.notificationTypes()[1]);
		});
	}

	PLUGIN_TEST_WITH_PREFIXED_TRAITS(CanPublishTransactionDependentNotifications, Default, _Default) {
		// Act:
		using Subscriber = mocks::MockTypedNotificationSubscriber<AccountPublicKeyNotification>;
		RunPublishTest<TTraits, Subscriber>([](const auto& transaction, const auto&, const auto& sub) {
			// Assert:
			ASSERT_EQ(1u, sub.numMatchingNotifications());
			EXPECT_EQ(transaction.SignerPublicKey, sub.matchingNotifications()[0].PublicKey);
		});
	}

	PLUGIN_TEST_WITH_PREFIXED_TRAITS(CanPublishContextDependentNotifications, Default, _Default) {
		// Act:
		using Subscriber = mocks::MockTypedNotificationSubscriber<mocks::MockAddressNotification>;
		RunPublishTest<TTraits, Subscriber>([](const auto&, const auto& context, const auto& sub) {
			// Assert:
			ASSERT_EQ(1u, sub.numMatchingNotifications());
			EXPECT_EQ(context.SignerAddress, sub.matchingNotifications()[0].Address);
		});
	}

	TEST(TEST_CLASS, PluginExposesCustomAdditionalRequiredCosignatories_OnlyEmbeddable) {
		// Arrange:
		auto pPlugin = OnlyEmbeddableEmbeddedTraits::CreatePlugin();

		OnlyEmbeddableEmbeddedTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);

		// Act:
		auto additionalCosignatories = pPlugin->additionalRequiredCosignatories(transaction);

		// Assert: cosignatories are forwarded from ExtractAdditionalRequiredCosignatories
		UnresolvedAddressSet expectedAdditionalCosignatories{
			UnresolvedAddress{ { 1 } },
			UnresolvedAddress{ { 2 } },
			mocks::GetRecipientAddress(transaction).copyTo<UnresolvedAddress>()
		};
		EXPECT_EQ(expectedAdditionalCosignatories, additionalCosignatories);
	}
}}
