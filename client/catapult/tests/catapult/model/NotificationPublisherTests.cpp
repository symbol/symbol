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

#include "catapult/model/NotificationPublisher.h"
#include "catapult/constants.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/nodeps/NumericTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS NotificationPublisherTests

	namespace {
		constexpr auto Plugin_Option_Flags = static_cast<mocks::PluginOptionFlags>(
				utils::to_underlying_type(mocks::PluginOptionFlags::Custom_Buffers) |
				utils::to_underlying_type(mocks::PluginOptionFlags::Publish_Custom_Notifications));

		template<typename TEntity, typename TAssertSubFunc>
		void PublishAll(const TEntity& entity, PublicationMode mode, TAssertSubFunc assertSub) {
			// Arrange:
			mocks::MockNotificationSubscriber sub;

			auto registry = mocks::CreateDefaultTransactionRegistry(Plugin_Option_Flags);
			auto pPub = CreateNotificationPublisher(registry, PublisherContext(), mode);

			// Act:
			auto hash = test::GenerateRandomData<Key_Size>();
			pPub->publish(model::WeakEntityInfo(entity, hash), sub);

			// Assert:
			assertSub(sub);
		}

		template<typename TEntity, typename TAssertSubFunc>
		void PublishAll(const TEntity& entity, TAssertSubFunc assertSub) {
			PublishAll(entity, PublicationMode::All, assertSub);
		}

		template<typename TNotification, typename TEntity, typename TAssertNotification>
		void PublishOne(const TEntity& entity, const Hash256& hash, TAssertNotification assertNotification) {
			// Arrange:
			mocks::MockTypedNotificationSubscriber<TNotification> sub;

			auto registry = mocks::CreateDefaultTransactionRegistry(Plugin_Option_Flags);
			auto pPub = CreateNotificationPublisher(registry, PublisherContext());

			// Act:
			pPub->publish(model::WeakEntityInfo(entity, hash), sub);

			// Assert:
			ASSERT_EQ(1u, sub.numMatchingNotifications());
			assertNotification(sub.matchingNotifications()[0]);
		}

		template<typename TNotification, typename TEntity, typename TAssertNotification>
		void PublishOne(const TEntity& entity, TAssertNotification assertNotification) {
			// Act:
			PublishOne<TNotification>(entity, test::GenerateRandomData<Hash256_Size>(), assertNotification);
		}
	}

	// region block

	TEST(TEST_CLASS, CanRaiseBlockAccountNotifications) {
		// Arrange:
		auto pBlock = test::GenerateEmptyRandomBlock();
		test::FillWithRandomData(pBlock->Signer);

		// Act:
		PublishAll(*pBlock, [&block = *pBlock](const auto& sub) {
			// Assert:
			EXPECT_EQ(4u, sub.numNotifications());
			EXPECT_EQ(0u, sub.numAddresses());
			EXPECT_EQ(1u, sub.numKeys());

			EXPECT_TRUE(sub.contains(block.Signer));
		});
	}

	TEST(TEST_CLASS, CanRaiseBlockEntityNotifications) {
		// Arrange:
		auto pBlock = test::GenerateEmptyRandomBlock();
		pBlock->Version = 0x1100;

		// Act:
		PublishOne<EntityNotification>(*pBlock, [](const auto& notification) {
			// Assert:
			EXPECT_EQ(static_cast<NetworkIdentifier>(0x11), notification.NetworkIdentifier);
		});
	}

	TEST(TEST_CLASS, CanRaiseBlockSignatureNotifications) {
		// Arrange:
		auto pBlock = test::GenerateEmptyRandomBlock();
		test::FillWithRandomData(pBlock->Signer);
		test::FillWithRandomData(pBlock->Signature);

		// Act:
		PublishOne<SignatureNotification>(*pBlock, [&block = *pBlock](const auto& notification) {
			// Assert:
			EXPECT_EQ(block.Signer, notification.Signer);
			EXPECT_EQ(block.Signature, notification.Signature);
			EXPECT_EQ(test::AsVoidPointer(&block.Version), test::AsVoidPointer(notification.Data.pData));
			EXPECT_EQ(sizeof(Block) - VerifiableEntity::Header_Size, notification.Data.Size);
		});
	}

	namespace {
		std::unique_ptr<model::Block> GenerateBlockWithTransactionFees(const std::vector<Amount>& fees) {
			auto pBlock = test::GenerateBlockWithTransactions(fees.size());
			test::FillWithRandomData(pBlock->Signer);

			auto i = 0u;
			for (auto& transaction : pBlock->Transactions())
				transaction.Fee = fees[i++];

			return pBlock;
		}
	}

	TEST(TEST_CLASS, CanRaiseBlockNotifications_BlockWithoutTransactions) {
		// Arrange:
		auto pBlock = GenerateBlockWithTransactionFees({});
		pBlock->Timestamp = Timestamp(123);
		pBlock->Difficulty = Difficulty(575);

		// Act:
		PublishOne<BlockNotification>(*pBlock, [&block = *pBlock](const auto& notification) {
			// Assert:
			EXPECT_EQ(block.Signer, notification.Signer);
			EXPECT_EQ(Timestamp(123), notification.Timestamp);
			EXPECT_EQ(Difficulty(575), notification.Difficulty);
			EXPECT_EQ(Amount(0), notification.TotalFee);
			EXPECT_EQ(0u, notification.NumTransactions);
		});
	}

	TEST(TEST_CLASS, CanRaiseBlockNotifications_BlockWithTransactions) {
		// Arrange:
		auto pBlock = GenerateBlockWithTransactionFees({ Amount(11), Amount(25), Amount(17) });
		pBlock->Timestamp = Timestamp(432);
		pBlock->Difficulty = Difficulty(575);

		// Act:
		PublishOne<BlockNotification>(*pBlock, [&block = *pBlock](const auto& notification) {
			// Assert:
			EXPECT_EQ(block.Signer, notification.Signer);
			EXPECT_EQ(Timestamp(432), notification.Timestamp);
			EXPECT_EQ(Difficulty(575), notification.Difficulty);
			EXPECT_EQ(Amount(53), notification.TotalFee);
			EXPECT_EQ(3u, notification.NumTransactions);
		});
	}

	TEST(TEST_CLASS, CanSuppressBasicBlockNotifications) {
		// Arrange:
		auto pBlock = GenerateBlockWithTransactionFees({});

		// Act:
		PublishAll(*pBlock, PublicationMode::Custom, [](const auto& sub) {
			// Assert: all notifications were suppressed (blocks do not have custom notifications)
			ASSERT_EQ(0u, sub.numNotifications());
		});
	}

	// endregion

	// region transaction

	TEST(TEST_CLASS, CanRaiseTransactionAccountNotifications) {
		// Arrange:
		auto pTransaction = mocks::CreateMockTransaction(0);
		test::FillWithRandomData(pTransaction->Signer);
		test::FillWithRandomData(pTransaction->Recipient);

		// Act:
		PublishAll(*pTransaction, [&transaction = *pTransaction](const auto& sub) {
			// Assert: both signer (from notification publisher) and recipient (from custom publish implementation) are raised
			EXPECT_EQ(0u, sub.numAddresses());
			EXPECT_EQ(2u, sub.numKeys());

			EXPECT_TRUE(sub.contains(transaction.Signer));
			EXPECT_TRUE(sub.contains(transaction.Recipient));
		});
	}

	TEST(TEST_CLASS, CanRaiseTransactionEntityNotifications) {
		// Arrange:
		auto pTransaction = mocks::CreateMockTransaction(0);
		pTransaction->Version = 0x1100;

		// Act:
		PublishOne<EntityNotification>(*pTransaction, [](const auto& notification) {
			// Assert:
			EXPECT_EQ(static_cast<NetworkIdentifier>(0x11), notification.NetworkIdentifier);
		});
	}

	TEST(TEST_CLASS, CanRaiseTransactionSignatureNotifications) {
		// Arrange:
		auto pTransaction = mocks::CreateMockTransaction(12);
		test::FillWithRandomData(pTransaction->Signer);
		test::FillWithRandomData(pTransaction->Signature);

		// Act:
		PublishOne<SignatureNotification>(*pTransaction, [&transaction = *pTransaction](const auto& notification) {
			// Assert:
			EXPECT_EQ(transaction.Signer, notification.Signer);
			EXPECT_EQ(transaction.Signature, notification.Signature);

			// - notice that mock plugin is configured with PluginOptionFlags::Custom_Buffers so dataBuffer() contains only data payload
			EXPECT_EQ(test::AsVoidPointer(&transaction + 1), test::AsVoidPointer(notification.Data.pData));
			EXPECT_EQ(12u, notification.Data.Size);
		});
	}

	TEST(TEST_CLASS, CanRaiseTransactionNotifications) {
		// Arrange:
		auto hash = test::GenerateRandomData<Hash256_Size>();
		auto pTransaction = mocks::CreateMockTransaction(12);
		test::FillWithRandomData(pTransaction->Signer);
		pTransaction->Deadline = Timestamp(454);

		// Act:
		PublishOne<TransactionNotification>(*pTransaction, hash, [&signer = pTransaction->Signer, &hash](const auto& notification) {
			// Assert:
			EXPECT_EQ(signer, notification.Signer);
			EXPECT_EQ(hash, notification.TransactionHash);
			EXPECT_EQ(static_cast<model::EntityType>(mocks::MockTransaction::Entity_Type), notification.TransactionType);
			EXPECT_EQ(Timestamp(454), notification.Deadline);
		});
	}

	TEST(TEST_CLASS, CanRaiseTransactionFeeNotifications) {
		// Arrange:
		auto hash = test::GenerateRandomData<Hash256_Size>();
		auto pTransaction = mocks::CreateMockTransaction(12);
		test::FillWithRandomData(pTransaction->Signer);
		pTransaction->Fee = Amount(765);

		// Act:
		PublishOne<BalanceDebitNotification>(*pTransaction, hash, [&signer = pTransaction->Signer](const auto& notification) {
			// Assert:
			EXPECT_EQ(signer, notification.Sender);
			EXPECT_EQ(Xem_Id, notification.MosaicId);
			EXPECT_EQ(Amount(765), notification.Amount);
		});
	}

	TEST(TEST_CLASS, CanRaiseCustomTransactionNotifications) {
		// Arrange:
		auto pTransaction = mocks::CreateMockTransaction(12);

		// Act:
		PublishAll(*pTransaction, [&transaction = *pTransaction](const auto& sub) {
			// Assert: 5 raised by NotificationPublisher, 8 raised by MockTransaction::publish (first is AccountPublicKeyNotification)
			ASSERT_EQ(5u + 1 + 7, sub.numNotifications());

			EXPECT_EQ(mocks::Mock_Observer_1_Notification, sub.notificationTypes()[6]);
			EXPECT_EQ(mocks::Mock_Validator_1_Notification, sub.notificationTypes()[7]);
			EXPECT_EQ(mocks::Mock_All_1_Notification, sub.notificationTypes()[8]);
			EXPECT_EQ(mocks::Mock_Observer_2_Notification, sub.notificationTypes()[9]);
			EXPECT_EQ(mocks::Mock_Validator_2_Notification, sub.notificationTypes()[10]);
			EXPECT_EQ(mocks::Mock_All_2_Notification, sub.notificationTypes()[11]);
			EXPECT_EQ(mocks::Mock_Hash_Notification, sub.notificationTypes()[12]);
		});
	}

	TEST(TEST_CLASS, CanRaiseCustomTransactionNotificationsDependentOnHash) {
		// Arrange:
		auto hash = test::GenerateRandomData<Hash256_Size>();
		auto pTransaction = mocks::CreateMockTransaction(12);

		// Act:
		PublishOne<mocks::HashNotification>(*pTransaction, hash, [&hash](const auto& notification) {
			// Assert:
			EXPECT_EQ(&hash, &notification.Hash);
		});
	}

	TEST(TEST_CLASS, CanSuppressCustomTransactionNotifications) {
		// Arrange:
		auto pTransaction = mocks::CreateMockTransaction(12);

		// Act:
		PublishAll(*pTransaction, PublicationMode::Basic, [&transaction = *pTransaction](const auto& sub) {
			// Assert: 5 raised by NotificationPublisher, none raised by MockTransaction::publish
			ASSERT_EQ(5u, sub.numNotifications());
			EXPECT_EQ(Core_Register_Account_Public_Key_Notification, sub.notificationTypes()[0]);
			EXPECT_EQ(Core_Entity_Notification, sub.notificationTypes()[1]);
			EXPECT_EQ(Core_Transaction_Notification, sub.notificationTypes()[2]);
			EXPECT_EQ(Core_Balance_Debit_Notification, sub.notificationTypes()[3]);
			EXPECT_EQ(Core_Signature_Notification, sub.notificationTypes()[4]);
		});
	}

	TEST(TEST_CLASS, CanSuppressBasicTransactionNotifications) {
		// Arrange:
		auto pTransaction = mocks::CreateMockTransaction(12);

		// Act:
		PublishAll(*pTransaction, PublicationMode::Custom, [&transaction = *pTransaction](const auto& sub) {
			// Assert: 8 raised by MockTransaction::publish (first is AccountPublicKeyNotification)
			ASSERT_EQ(1u + 7, sub.numNotifications());

			EXPECT_EQ(Core_Register_Account_Public_Key_Notification, sub.notificationTypes()[0]);
			EXPECT_EQ(mocks::Mock_Observer_1_Notification, sub.notificationTypes()[1]);
			EXPECT_EQ(mocks::Mock_Validator_1_Notification, sub.notificationTypes()[2]);
			EXPECT_EQ(mocks::Mock_All_1_Notification, sub.notificationTypes()[3]);
			EXPECT_EQ(mocks::Mock_Observer_2_Notification, sub.notificationTypes()[4]);
			EXPECT_EQ(mocks::Mock_Validator_2_Notification, sub.notificationTypes()[5]);
			EXPECT_EQ(mocks::Mock_All_2_Notification, sub.notificationTypes()[6]);
			EXPECT_EQ(mocks::Mock_Hash_Notification, sub.notificationTypes()[7]);
		});
	}

	// endregion

	// region other

	TEST(TEST_CLASS, CanRaiseOtherAccountNotifications) {
		// Arrange:
		VerifiableEntity entity{};
		test::FillWithRandomData(entity.Signer);

		// Act:
		PublishAll(entity, [&entity](const auto& sub) {
			// Assert:
			EXPECT_EQ(2u, sub.numNotifications());
			EXPECT_EQ(0u, sub.numAddresses());
			EXPECT_EQ(1u, sub.numKeys());

			EXPECT_TRUE(sub.contains(entity.Signer));
		});
	}

	TEST(TEST_CLASS, CanRaiseOtherEntityNotifications) {
		// Arrange:
		VerifiableEntity entity{};
		entity.Version = 0x1100;

		// Act:
		PublishOne<EntityNotification>(entity, [](const auto& notification) {
			// Assert:
			EXPECT_EQ(static_cast<NetworkIdentifier>(0x11), notification.NetworkIdentifier);
		});
	}

	TEST(TEST_CLASS, CanSuppressBasicOtherNotifications) {
		// Arrange:
		VerifiableEntity entity{};
		test::FillWithRandomData(entity.Signer);

		// Act:
		PublishAll(entity, PublicationMode::Custom, [](const auto& sub) {
			// Assert: all notifications were suppressed (other entities do not have custom notifications)
			ASSERT_EQ(0u, sub.numNotifications());
		});
	}

	namespace {
		PublisherContext CreateCustomPublisherContext() {
			return PublisherContext(
					[](auto mosaicId) { return MosaicId(mosaicId.unwrap() + 10); },
					[](const auto& address) { return test::CopyAndXorArray(extensions::CopyToAddress(address)); });
		}
	}

	TEST(TEST_CLASS, CanRaiseResolvedNotifications) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<BalanceTransferNotification> sub;

		auto pTransaction = mocks::CreateTransactionWithFeeAndTransfers(Amount(), {
			{ MosaicId(123), Amount(1111) }
		});
		auto recipientAddress = model::PublicKeyToAddress(pTransaction->Recipient, model::NetworkIdentifier::Mijin_Test);

		auto registry = mocks::CreateDefaultTransactionRegistry(mocks::PluginOptionFlags::Publish_Transfers);
		auto pPub = CreateNotificationPublisher(registry, CreateCustomPublisherContext());

		// Act:
		pPub->publish(model::WeakEntityInfo(*pTransaction, Hash256()), sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		EXPECT_EQ(MosaicId(133), notification.MosaicId);
		EXPECT_EQ(test::CopyAndXorArray(recipientAddress), notification.Recipient);
	}

	// endregion
}}
