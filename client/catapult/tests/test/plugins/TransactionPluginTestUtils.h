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
#include "TransactionPluginTests.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"

namespace catapult { namespace test {

	/// Helpers for testing TransactionPlugin::publish.
	template<typename TTraits>
	class TransactionPluginTestUtils {
	private:
		using TransactionType = typename TTraits::TransactionType;

	public:
		template<typename... TArgs>
		static void AssertNotificationTypes(
				const TransactionType& transaction,
				const std::vector<model::NotificationType>& expectedNotificationTypes,
				TArgs&& ...args) {
			// Arrange:
			mocks::MockNotificationSubscriber sub;
			auto pPlugin = TTraits::CreatePlugin(std::forward<TArgs>(args)...);

			// Act:
			PublishTransaction(*pPlugin, transaction, sub);

			// Assert:
			const auto& notificationTypes = sub.notificationTypes();
			ASSERT_EQ(expectedNotificationTypes.size(), notificationTypes.size());
			for (auto i = 0u; i < expectedNotificationTypes.size(); ++i)
				EXPECT_EQ(expectedNotificationTypes[i], notificationTypes[i]) << "notification type at " << i;
		}

	public:
		class PublishTestBuilder {
		public:
			template<typename TNotification>
			void addExpectation(const consumer<TNotification>& expectation) {
				addExpectation(0, expectation);
			}

			template<typename TNotification>
			void addExpectation(size_t notificationIndex, const consumer<TNotification>& expectation) {
				auto pSubscriber = std::make_unique<mocks::MockTypedNotificationSubscriber<TNotification>>();
				m_expectations.push_back([notificationIndex, expectation, &subscriber = *pSubscriber]() {
					ASSERT_LT(notificationIndex, subscriber.numMatchingNotifications());
					expectation(subscriber.matchingNotifications()[notificationIndex]);
				});
				m_subscribers.push_back(std::move(pSubscriber));
			}

		public:
			template<typename... TArgs>
			void runTest(const TransactionType& transaction, TArgs&& ...args) {
				runTestHelper(transaction, std::forward<TArgs>(args)...);
			}

			template<typename... TArgs>
			void runTestWithHash(const TransactionType& transaction, const Hash256& transactionHash, TArgs&& ...args) {
				runTestHelper(model::WeakEntityInfoT<model::Transaction>(transaction, transactionHash), std::forward<TArgs>(args)...);
			}

		private:
			template<typename TTransactionProxy, typename... TArgs>
			void runTestHelper(const TTransactionProxy& transaction, TArgs&& ...args) {
				auto pPlugin = TTraits::CreatePlugin(std::forward<TArgs>(args)...);

				for (const auto& pSubscriber : m_subscribers)
					TransactionPluginTestUtils<TTraits>::PublishTransaction(*pPlugin, transaction, *pSubscriber);

				for (const auto& expectation : m_expectations)
					expectation();
			}

		private:
			std::vector<std::unique_ptr<model::NotificationSubscriber>> m_subscribers;
			std::vector<action> m_expectations;
		};

	private:
		template<typename TTransaction>
		static model::PublishContext CreatePublishContext(const TTransaction& transaction) {
			model::PublishContext context;
			context.SignerAddress = model::GetSignerAddress(transaction);
			return context;
		}

		static void PublishTransaction(
				const model::TransactionPlugin& plugin,
				const TransactionType& transaction,
				model::NotificationSubscriber& sub) {
			plugin.publish({ transaction, Hash256() }, CreatePublishContext(transaction), sub);
		}

		static void PublishTransaction(
				const model::TransactionPlugin& plugin,
				const model::WeakEntityInfoT<model::Transaction>& transactionInfo,
				model::NotificationSubscriber& sub) {
			plugin.publish(transactionInfo, CreatePublishContext(transactionInfo.entity()), sub);
		}

		static void PublishTransaction(
				const model::EmbeddedTransactionPlugin& plugin,
				const TransactionType& transaction,
				model::NotificationSubscriber& sub) {
			plugin.publish(transaction, CreatePublishContext(transaction), sub);
		}
	};
}}
