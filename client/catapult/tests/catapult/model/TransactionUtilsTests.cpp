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

#include "catapult/model/TransactionUtils.h"
#include "catapult/model/Address.h"
#include "catapult/model/NotificationPublisher.h"
#include "catapult/model/NotificationSubscriber.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS TransactionUtilsTests

	namespace {
		constexpr auto Network_Identifier = NetworkIdentifier::Mijin_Test;

		class MockNotificationPublisher : public NotificationPublisher {
		public:
			enum class Mode { Address, Public_Key, Other };

		public:
			explicit MockNotificationPublisher(Mode mode) : m_mode(mode)
			{}

		public:
			void publish(const WeakEntityInfo& entityInfo, NotificationSubscriber& sub) const override {
				const auto& transaction = entityInfo.cast<mocks::MockTransaction>().entity();

				if (Mode::Address == m_mode) {
					auto senderAddress = PublicKeyToAddress(transaction.Signer, Network_Identifier);
					auto recipientAddress = PublicKeyToAddress(transaction.Recipient, Network_Identifier);
					sub.notify(AccountAddressNotification(senderAddress));
					sub.notify(AccountAddressNotification(recipientAddress));
				} else if (Mode::Public_Key == m_mode) {
					sub.notify(AccountPublicKeyNotification(transaction.Signer));
					sub.notify(AccountPublicKeyNotification(transaction.Recipient));
				} else {
					sub.notify(EntityNotification(transaction.Network()));
				}
			}

		private:
			Mode m_mode;
		};

		void RunExtractAddressesTest(MockNotificationPublisher::Mode mode) {
			// Arrange:
			auto pTransaction = mocks::CreateMockTransactionWithSignerAndRecipient(
					test::GenerateRandomData<Key_Size>(),
					test::GenerateRandomData<Key_Size>());
			auto senderAddress = PublicKeyToAddress(pTransaction->Signer, Network_Identifier);
			auto recipientAddress = PublicKeyToAddress(pTransaction->Recipient, Network_Identifier);

			MockNotificationPublisher notificationPublisher(mode);

			// Act:
			auto addresses = ExtractAddresses(*pTransaction, notificationPublisher);

			// Assert:
			EXPECT_EQ(2u, addresses.size());
			EXPECT_TRUE(addresses.cend() != addresses.find(senderAddress));
			EXPECT_TRUE(addresses.cend() != addresses.find(recipientAddress));
		}
	}

	TEST(TEST_CLASS, ExtractAddressesExtractsAddressesFromAddressNotifications) {
		// Assert:
		RunExtractAddressesTest(MockNotificationPublisher::Mode::Address);
	}

	TEST(TEST_CLASS, ExtractAddressesExtractsAddressesFromPublicKeyNotifications) {
		// Assert:
		RunExtractAddressesTest(MockNotificationPublisher::Mode::Public_Key);
	}

	TEST(TEST_CLASS, ExtractAddressesDoesNotExtractAddressesFromOtherNotifications) {
		// Arrange:
		auto pTransaction = mocks::CreateMockTransactionWithSignerAndRecipient(
				test::GenerateRandomData<Key_Size>(),
				test::GenerateRandomData<Key_Size>());

		MockNotificationPublisher notificationPublisher(MockNotificationPublisher::Mode::Other);

		// Act:
		auto addresses = ExtractAddresses(*pTransaction, notificationPublisher);

		// Assert:
		EXPECT_TRUE(addresses.empty());
	}
}}
