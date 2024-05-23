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
#include "TransactionPluginTests.h"
#include "catapult/model/Block.h"
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
				TArgs&&... args) {
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
			template<typename TTransactionProxy, typename... TArgs>
			void runTest(const TTransactionProxy& transaction, TArgs&&... args) const {
				runTestHelper(transaction, std::forward<TArgs>(args)...);
			}

			template<typename... TArgs>
			void runTestWithHash(const TransactionType& transaction, const Hash256& transactionHash, TArgs&&... args) const {
				runTestHelper(model::WeakEntityInfoT<model::Transaction>(transaction, transactionHash), std::forward<TArgs>(args)...);
			}

		private:
			template<typename TTransactionProxy, typename... TArgs>
			void runTestHelper(const TTransactionProxy& transaction, TArgs&&... args) const {
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

		template<typename TTransaction>
		static model::PublishContext CreatePublishContext(const model::WeakEntityInfoT<TTransaction>& transactionInfo) {
			auto context = CreatePublishContext(transactionInfo.entity());

			if (transactionInfo.isAssociatedBlockHeaderSet())
				context.BlockHeight = transactionInfo.associatedBlockHeader().Height;

			return context;
		}

		static model::WeakEntityInfoT<model::Transaction> ToTransactionInfo(
				const model::WeakEntityInfoT<model::Transaction>& transactionInfo) {
			return transactionInfo;
		}

		static model::WeakEntityInfoT<model::Transaction> ToTransactionInfo(const TransactionType& transaction) {
			return { transaction, Hash256() };
		}

		template<typename TTransactionProxy>
		static void PublishTransaction(
				const model::TransactionPlugin& plugin,
				const TTransactionProxy& transaction,
				model::NotificationSubscriber& sub) {
			plugin.publish(ToTransactionInfo(transaction), CreatePublishContext(transaction), sub);
		}

		static void PublishTransaction(
				const model::EmbeddedTransactionPlugin& plugin,
				const TransactionType& transaction,
				model::NotificationSubscriber& sub) {
			plugin.publish(transaction, CreatePublishContext(transaction), sub);
		}

		// note: this is a bit of an abuse because WeakEntityInfoT<EmbeddedTransaction> is never used,
		//       but it indirectly allows setting BlockHeight in PublishContext
		static void PublishTransaction(
				const model::EmbeddedTransactionPlugin& plugin,
				const model::WeakEntityInfoT<model::EmbeddedTransaction>& transactionInfo,
				model::NotificationSubscriber& sub) {
			plugin.publish(transactionInfo.entity(), CreatePublishContext(transactionInfo), sub);
		}
	};
}}
