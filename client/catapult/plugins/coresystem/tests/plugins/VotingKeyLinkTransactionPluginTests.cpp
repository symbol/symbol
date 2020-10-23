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

#include "src/plugins/VotingKeyLinkTransactionPlugin.h"
#include "src/model/KeyLinkNotifications.h"
#include "src/model/VotingKeyLinkTransaction.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS VotingKeyLinkTransactionPluginTests

	// region test utils

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(VotingKeyLink, 1, 2,)

		struct V1RegularTraits {
			using TransactionType = model::VotingKeyLinkV1Transaction;

			static constexpr auto Min_Supported_Version = 1;
			static constexpr auto Max_Supported_Version = 2;

			static void SetVersion(TransactionType& transaction) {
				transaction.Version = 1;
			}

			static auto CreatePlugin() {
				return CreateVotingKeyLinkTransactionPlugin();
			}
		};

		struct V1EmbeddedTraits {
			using TransactionType = model::EmbeddedVotingKeyLinkV1Transaction;

			static constexpr auto Min_Supported_Version = 1;
			static constexpr auto Max_Supported_Version = 2;

			static void SetVersion(TransactionType& transaction) {
				transaction.Version = 1;
			}

			static auto CreatePlugin() {
				return test::ExtractEmbeddedPlugin(V1RegularTraits::CreatePlugin());
			}
		};

#define MULTI_VERSION_PLUGIN_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Regular_V1) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<V1RegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Embedded_V1) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<V1EmbeddedTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Regular_V2) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RegularTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Embedded_V2) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<EmbeddedTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , Entity_Type_Voting_Key_Link)
	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, V1, V1, Entity_Type_Voting_Key_Link)

	// endregion

	// region publish - action link

	namespace {
		template<typename TTraits>
		void AddCommonExpectations(
				typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder& builder,
				const typename TTraits::TransactionType& transaction) {
			builder.template addExpectation<KeyLinkActionNotification>([&transaction](const auto& notification) {
				EXPECT_EQ(transaction.LinkAction, notification.LinkAction);
			});
			builder.template addExpectation<VotingKeyLinkNotification>([&transaction](const auto& notification) {
				EXPECT_EQ(transaction.SignerPublicKey, notification.MainAccountPublicKey);
				EXPECT_EQ(transaction.LinkedPublicKey, notification.LinkedPublicKey.VotingKey);
				EXPECT_EQ(transaction.StartEpoch, notification.LinkedPublicKey.StartEpoch);
				EXPECT_EQ(transaction.EndEpoch, notification.LinkedPublicKey.EndEpoch);
				EXPECT_EQ(transaction.LinkAction, notification.LinkAction);
			});
		}
	}

	MULTI_VERSION_PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenLinkActionIsLink) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		TTraits::SetVersion(transaction);
		transaction.LinkAction = LinkAction::Link;

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(transaction, {
			KeyLinkActionNotification::Notification_Type,
			VotingKeyLinkNotification::Notification_Type
		});
	}

	MULTI_VERSION_PLUGIN_TEST(CanPublishAllNotificationsWhenLinkActionIsLink) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		TTraits::SetVersion(transaction);
		transaction.LinkAction = LinkAction::Link;

		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		AddCommonExpectations<TTraits>(builder, transaction);

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion

	// region publish - action unlink

	MULTI_VERSION_PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenLinkActionIsUnlink) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		TTraits::SetVersion(transaction);
		transaction.LinkAction = LinkAction::Unlink;

		// Act + Assert:
		test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(transaction, {
			KeyLinkActionNotification::Notification_Type,
			VotingKeyLinkNotification::Notification_Type
		});
	}

	MULTI_VERSION_PLUGIN_TEST(CanPublishAllNotificationsWhenLinkActionIsUnlink) {
		// Arrange:
		typename TTraits::TransactionType transaction;
		test::FillWithRandomData(transaction);
		TTraits::SetVersion(transaction);
		transaction.LinkAction = LinkAction::Unlink;

		typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
		AddCommonExpectations<TTraits>(builder, transaction);

		// Act + Assert:
		builder.runTest(transaction);
	}

	// endregion
}}
