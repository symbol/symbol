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
		void Publish(const TTransaction& transaction, NotificationSubscriber& sub) {
			// raise a notification dependent on the transaction data
			sub.notify(test::CreateBlockNotification(transaction.Signer));
		}

		template<TransactionPluginFactoryOptions Options>
		struct RegularTraits {
			using TransactionType = mocks::MockTransaction;
			static constexpr auto Min_Supported_Version = TransactionType::Current_Version;
			static constexpr auto Max_Supported_Version = TransactionType::Current_Version;

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

	PLUGIN_TEST_WITH_PREFIXED_TRAITS(CanPublishNotifications, Default, _Default) {
		// Arrange:
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		mocks::MockTypedNotificationSubscriber<BlockNotification> sub;

		// Act:
		pPlugin->publish(transaction, sub);

		// Assert:
		EXPECT_EQ(1u, sub.numNotifications());
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		EXPECT_EQ(transaction.Signer, sub.matchingNotifications()[0].Signer);
	}

	TEST(TEST_CLASS, PluginExposesCustomAdditionalRequiredCosigners_OnlyEmbeddable) {
		// Arrange:
		auto pPlugin = OnlyEmbeddableEmbeddedTraits::CreatePlugin();

		OnlyEmbeddableEmbeddedTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);

		// Act:
		auto additionalCosigners = pPlugin->additionalRequiredCosigners(transaction);

		// Assert:
		utils::KeySet expectedAdditionalCosigners{ Key{ { 1 } }, Key{ { 2 } }, transaction.Recipient };
		EXPECT_EQ(expectedAdditionalCosigners, additionalCosigners);
	}
}}
