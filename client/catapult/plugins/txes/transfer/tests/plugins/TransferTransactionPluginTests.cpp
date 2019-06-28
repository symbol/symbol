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

#include "src/plugins/TransferTransactionPlugin.h"
#include "sdk/src/extensions/ConversionExtensions.h"
#include "src/model/TransferNotifications.h"
#include "src/model/TransferTransaction.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS TransferTransactionPluginTests

	namespace {
		DEFINE_TRANSACTION_PLUGIN_TEST_TRAITS(Transfer, 1, 1,)

		template<typename TTraits>
		auto CreateTransactionWithMosaics(uint8_t numMosaics, uint16_t messageSize = 0) {
			using TransactionType = typename TTraits::TransactionType;
			uint32_t entitySize = sizeof(TransactionType) + numMosaics * sizeof(Mosaic) + messageSize;
			auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
			pTransaction->Size = entitySize;
			pTransaction->MessageSize = messageSize;
			pTransaction->MosaicsCount = numMosaics;
			test::FillWithRandomData(pTransaction->Signer);
			test::FillWithRandomData(pTransaction->Recipient);
			return pTransaction;
		}
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, , , Entity_Type_Transfer)

	PLUGIN_TEST(CanCalculateSize) {
		// Arrange:
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction;
		transaction.Size = 0;
		transaction.MessageSize = 100;
		transaction.MosaicsCount = 7;

		// Act:
		auto realSize = pPlugin->calculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(sizeof(typename TTraits::TransactionType) + 100 + 7 * sizeof(Mosaic), realSize);
	}

	PLUGIN_TEST(CanExtractAccounts) {
		// Arrange:
		mocks::MockNotificationSubscriber sub;
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction;
		transaction.MessageSize = 0;
		transaction.MosaicsCount = 0;
		test::FillWithRandomData(transaction.Signer);
		test::FillWithRandomData(transaction.Recipient);

		// Act:
		test::PublishTransaction(*pPlugin, transaction, sub);

		// Assert:
		EXPECT_EQ(2u, sub.numNotifications());
		EXPECT_EQ(1u, sub.numAddresses());
		EXPECT_EQ(0u, sub.numKeys());

		EXPECT_TRUE(sub.contains(transaction.Recipient));
	}

	PLUGIN_TEST(CanExtractAddressInteraction) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<AddressInteractionNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();

		typename TTraits::TransactionType transaction;
		transaction.MessageSize = 0;
		transaction.MosaicsCount = 0;
		test::FillWithRandomData(transaction.Signer);
		test::FillWithRandomData(transaction.Recipient);
		transaction.Type = static_cast<model::EntityType>(0x0815);

		// Act:
		test::PublishTransaction(*pPlugin, transaction, sub);

		// Assert:
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		const auto& notification = sub.matchingNotifications()[0];
		EXPECT_EQ(transaction.Signer, notification.Source);
		EXPECT_EQ(transaction.Type, notification.TransactionType);
		EXPECT_EQ(model::UnresolvedAddressSet{ transaction.Recipient }, notification.ParticipantsByAddress);
		EXPECT_EQ(utils::KeySet(), notification.ParticipantsByKey);
	}

	// region balance change

	PLUGIN_TEST(CanExtractZeroTransfers) {
		// Arrange:
		mocks::MockNotificationSubscriber sub;
		auto pPlugin = TTraits::CreatePlugin();

		auto pTransaction = CreateTransactionWithMosaics<TTraits>(0);

		// Act:
		test::PublishTransaction(*pPlugin, *pTransaction, sub);

		// Assert:
		EXPECT_EQ(2u, sub.numNotifications());
		EXPECT_EQ(0u, sub.numTransfers());
	}

	PLUGIN_TEST(CanExtractSingleTransfer) {
		// Arrange:
		mocks::MockNotificationSubscriber sub;
		mocks::MockTypedNotificationSubscriber<TransferMosaicsNotification> mosaicsSub;
		auto pPlugin = TTraits::CreatePlugin();

		auto pTransaction = CreateTransactionWithMosaics<TTraits>(1);
		*pTransaction->MosaicsPtr() = { UnresolvedMosaicId(123), Amount(9876) };

		// Act:
		test::PublishTransaction(*pPlugin, *pTransaction, sub);
		test::PublishTransaction(*pPlugin, *pTransaction, mosaicsSub);

		// Assert:
		EXPECT_EQ(4u, sub.numNotifications());
		EXPECT_EQ(1u, sub.numTransfers());
		EXPECT_TRUE(sub.contains(pTransaction->Signer, pTransaction->Recipient, UnresolvedMosaicId(123), Amount(9876)));

		ASSERT_EQ(1u, mosaicsSub.numMatchingNotifications());
		EXPECT_EQ(1u, mosaicsSub.matchingNotifications()[0].MosaicsCount);
		EXPECT_EQ(pTransaction->MosaicsPtr(), mosaicsSub.matchingNotifications()[0].MosaicsPtr);
	}

	PLUGIN_TEST(CanExtractMultipleTransfers) {
		// Arrange:
		mocks::MockNotificationSubscriber sub;
		mocks::MockTypedNotificationSubscriber<TransferMosaicsNotification> mosaicsSub;
		auto pPlugin = TTraits::CreatePlugin();

		auto pTransaction = CreateTransactionWithMosaics<TTraits>(3);
		auto pMosaic = pTransaction->MosaicsPtr();
		*pMosaic++ = { UnresolvedMosaicId(123), Amount(9876) };
		*pMosaic++ = { UnresolvedMosaicId(777), Amount(444) };
		*pMosaic++ = { UnresolvedMosaicId(625), Amount(25) };

		// Act:
		test::PublishTransaction(*pPlugin, *pTransaction, sub);
		test::PublishTransaction(*pPlugin, *pTransaction, mosaicsSub);

		// Assert:
		EXPECT_EQ(6u, sub.numNotifications());
		EXPECT_EQ(3u, sub.numTransfers());
		EXPECT_TRUE(sub.contains(pTransaction->Signer, pTransaction->Recipient, UnresolvedMosaicId(123), Amount(9876)));
		EXPECT_TRUE(sub.contains(pTransaction->Signer, pTransaction->Recipient, UnresolvedMosaicId(777), Amount(444)));
		EXPECT_TRUE(sub.contains(pTransaction->Signer, pTransaction->Recipient, UnresolvedMosaicId(625), Amount(25)));

		ASSERT_EQ(1u, mosaicsSub.numMatchingNotifications());
		EXPECT_EQ(3u, mosaicsSub.matchingNotifications()[0].MosaicsCount);
		EXPECT_EQ(pTransaction->MosaicsPtr(), mosaicsSub.matchingNotifications()[0].MosaicsPtr);
	}

	// endregion

	PLUGIN_TEST(CanExtractMessage) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<TransferMessageNotification> sub;
		auto pPlugin = TTraits::CreatePlugin();

		auto pTransaction = CreateTransactionWithMosaics<TTraits>(0, 17);

		// Act:
		test::PublishTransaction(*pPlugin, *pTransaction, sub);

		// Assert:
		EXPECT_EQ(3u, sub.numNotifications());
		ASSERT_EQ(1u, sub.numMatchingNotifications());
		EXPECT_EQ(17u, sub.matchingNotifications()[0].MessageSize);
	}
}}
