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
