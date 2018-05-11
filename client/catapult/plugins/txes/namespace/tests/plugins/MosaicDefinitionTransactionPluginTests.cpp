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

#include "src/plugins/MosaicDefinitionTransactionPlugin.h"
#include "src/model/MosaicDefinitionTransaction.h"
#include "src/model/MosaicNotifications.h"
#include "catapult/utils/MemoryUtils.h"
#include "catapult/constants.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS MosaicDefinitionTransactionPluginTests

	namespace {
		TRANSACTION_PLUGIN_WITH_CONFIG_TEST_TRAITS(MosaicDefinition, MosaicRentalFeeConfiguration)

		MosaicRentalFeeConfiguration CreateRentalFeeConfiguration(Amount fee) {
			return {
				test::GenerateRandomData<Key_Size>(),
				test::GenerateRandomData<Address_Decoded_Size>(),
				fee,
				test::GenerateRandomData<Key_Size>()
			};
		}
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(TEST_CLASS, Entity_Type_Mosaic_Definition, CreateRentalFeeConfiguration(Amount(0)))

	PLUGIN_TEST(CanCalculateSize) {
		// Arrange:
		auto pPlugin = TTraits::CreatePlugin(CreateRentalFeeConfiguration(Amount(0)));

		typename TTraits::TransactionType transaction{};
		transaction.Size = 0;
		transaction.MosaicNameSize = 100;
		transaction.PropertiesHeader.Count = 2;

		// Act:
		auto realSize = pPlugin->calculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(sizeof(typename TTraits::TransactionType) + 100 + 2 * sizeof(MosaicProperty), realSize);
	}

	PLUGIN_TEST(CanExtractAccounts) {
		// Arrange:
		mocks::MockNotificationSubscriber sub;
		auto config = CreateRentalFeeConfiguration(Amount(0));
		auto pPlugin = TTraits::CreatePlugin(config);

		typename TTraits::TransactionType transaction{};
		test::FillWithRandomData(transaction.Signer);

		// Act:
		pPlugin->publish(transaction, sub);

		// Assert:
		EXPECT_EQ(5u, sub.numNotifications());
		EXPECT_EQ(0u, sub.numAddresses());
		EXPECT_EQ(1u, sub.numKeys());

		EXPECT_TRUE(sub.contains(config.SinkPublicKey));
	}

	// region balance change

	namespace {
		template<typename TTraits, typename TAssertTransfers>
		void RunBalanceChangeObserverTest(bool isSignerExempt, TAssertTransfers assertTransfers) {
			// Arrange:
			mocks::MockNotificationSubscriber sub;
			auto config = CreateRentalFeeConfiguration(Amount(987));
			auto pPlugin = TTraits::CreatePlugin(config);

			// - prepare the transaction
			typename TTraits::TransactionType transaction{};
			test::FillWithRandomData(transaction.Signer);
			if (isSignerExempt)
				transaction.Signer = config.NemesisPublicKey;

			// Act:
			pPlugin->publish(transaction, sub);

			// Assert:
			assertTransfers(sub, transaction.Signer, config.SinkAddress);
		}
	}

	PLUGIN_TEST(RentalFeeIsExtractedFromNonNemesis) {
		// Arrange:
		RunBalanceChangeObserverTest<TTraits>(false, [](const auto& sub, const auto& signer, const auto& recipient) {
			// Assert:
			EXPECT_EQ(5u, sub.numNotifications());
			EXPECT_EQ(1u, sub.numTransfers());
			EXPECT_TRUE(sub.contains(signer, recipient, Xem_Id, Amount(987)));
		});
	}

	PLUGIN_TEST(RentalFeeIsExemptedFromNemesis) {
		// Arrange:
		RunBalanceChangeObserverTest<TTraits>(true, [](const auto& sub, const auto&, const auto&) {
			// Assert:
			EXPECT_EQ(4u, sub.numNotifications());
			EXPECT_EQ(0u, sub.numTransfers());
		});
	}

	// endregion

	// region registration

	template<typename TTraits>
	auto CreateTransactionWithPropertiesAndName(uint8_t numProperties, uint8_t nameSize) {
		using TransactionType = typename TTraits::TransactionType;
		uint32_t entitySize = sizeof(TransactionType) + numProperties * sizeof(MosaicProperty) + nameSize;
		auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
		pTransaction->Size = entitySize;
		pTransaction->PropertiesHeader.Count = numProperties;
		pTransaction->MosaicNameSize = nameSize;
		test::FillWithRandomData(pTransaction->Signer);
		return pTransaction;
	}

	PLUGIN_TEST(CanExtractDefinitionNotificationsWhenOptionalPropertiesArePresent) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<MosaicNameNotification> mosaicNameSub;
		mocks::MockTypedNotificationSubscriber<MosaicPropertiesNotification> mosaicPropertiesSub;
		mocks::MockTypedNotificationSubscriber<MosaicDefinitionNotification> mosaicDefinitionSub;
		auto config = CreateRentalFeeConfiguration(Amount(0));
		auto pPlugin = TTraits::CreatePlugin(config);

		auto pTransaction = CreateTransactionWithPropertiesAndName<TTraits>(1, 12);
		pTransaction->ParentId = NamespaceId(768);
		pTransaction->MosaicId = MosaicId(123);
		pTransaction->PropertiesHeader.Flags = static_cast<MosaicFlags>(2);
		pTransaction->PropertiesHeader.Divisibility = 7;
		pTransaction->PropertiesPtr()[0] = { MosaicPropertyId::Duration, 5 };

		// Act:
		pPlugin->publish(*pTransaction, mosaicNameSub);
		pPlugin->publish(*pTransaction, mosaicPropertiesSub);
		pPlugin->publish(*pTransaction, mosaicDefinitionSub);

		// Assert:
		ASSERT_EQ(1u, mosaicNameSub.numMatchingNotifications());
		EXPECT_EQ(MosaicId(123), mosaicNameSub.matchingNotifications()[0].MosaicId);
		EXPECT_EQ(NamespaceId(768), mosaicNameSub.matchingNotifications()[0].ParentId);
		EXPECT_EQ(12u, mosaicNameSub.matchingNotifications()[0].NameSize);
		EXPECT_EQ(pTransaction->NamePtr(), mosaicNameSub.matchingNotifications()[0].NamePtr);

		ASSERT_EQ(1u, mosaicPropertiesSub.numMatchingNotifications());
		EXPECT_EQ(&pTransaction->PropertiesHeader, &mosaicPropertiesSub.matchingNotifications()[0].PropertiesHeader);
		EXPECT_EQ(pTransaction->PropertiesPtr(), mosaicPropertiesSub.matchingNotifications()[0].PropertiesPtr);

		ASSERT_EQ(1u, mosaicDefinitionSub.numMatchingNotifications());
		EXPECT_EQ(pTransaction->Signer, mosaicDefinitionSub.matchingNotifications()[0].Signer);
		EXPECT_EQ(NamespaceId(768), mosaicDefinitionSub.matchingNotifications()[0].ParentId);
		EXPECT_EQ(MosaicId(123), mosaicDefinitionSub.matchingNotifications()[0].MosaicId);

		auto expectedProperties = MosaicProperties::FromValues({ { 2, 7, 5 } });
		EXPECT_EQ(expectedProperties, mosaicDefinitionSub.matchingNotifications()[0].Properties);
	}

	PLUGIN_TEST(CanExtractDefinitionNotificationsWhenNoOptionalPropertiesArePresent) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<MosaicNameNotification> mosaicNameSub;
		mocks::MockTypedNotificationSubscriber<MosaicPropertiesNotification> mosaicPropertiesSub;
		mocks::MockTypedNotificationSubscriber<MosaicDefinitionNotification> mosaicDefinitionSub;
		auto config = CreateRentalFeeConfiguration(Amount(0));
		auto pPlugin = TTraits::CreatePlugin(config);

		auto pTransaction = CreateTransactionWithPropertiesAndName<TTraits>(0, 12);
		pTransaction->ParentId = NamespaceId(768);
		pTransaction->MosaicId = MosaicId(123);
		pTransaction->PropertiesHeader.Flags = static_cast<MosaicFlags>(2);
		pTransaction->PropertiesHeader.Divisibility = 7;

		// Act:
		pPlugin->publish(*pTransaction, mosaicNameSub);
		pPlugin->publish(*pTransaction, mosaicPropertiesSub);
		pPlugin->publish(*pTransaction, mosaicDefinitionSub);

		// Assert:
		ASSERT_EQ(1u, mosaicNameSub.numMatchingNotifications());
		EXPECT_EQ(MosaicId(123), mosaicNameSub.matchingNotifications()[0].MosaicId);
		EXPECT_EQ(NamespaceId(768), mosaicNameSub.matchingNotifications()[0].ParentId);
		EXPECT_EQ(12u, mosaicNameSub.matchingNotifications()[0].NameSize);
		EXPECT_EQ(pTransaction->NamePtr(), mosaicNameSub.matchingNotifications()[0].NamePtr);

		ASSERT_EQ(1u, mosaicPropertiesSub.numMatchingNotifications());
		EXPECT_EQ(&pTransaction->PropertiesHeader, &mosaicPropertiesSub.matchingNotifications()[0].PropertiesHeader);
		EXPECT_FALSE(!!mosaicPropertiesSub.matchingNotifications()[0].PropertiesPtr);

		ASSERT_EQ(1u, mosaicDefinitionSub.numMatchingNotifications());
		EXPECT_EQ(pTransaction->Signer, mosaicDefinitionSub.matchingNotifications()[0].Signer);
		EXPECT_EQ(NamespaceId(768), mosaicDefinitionSub.matchingNotifications()[0].ParentId);
		EXPECT_EQ(MosaicId(123), mosaicDefinitionSub.matchingNotifications()[0].MosaicId);

		auto expectedProperties = MosaicProperties::FromValues({ { 2, 7, 0 } });
		EXPECT_EQ(expectedProperties, mosaicDefinitionSub.matchingNotifications()[0].Properties);
	}

	PLUGIN_TEST(CanExtractMosaicNameNotificationWhenThereIsNoName) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<MosaicNameNotification> mosaicNameSub;
		auto config = CreateRentalFeeConfiguration(Amount(0));
		auto pPlugin = TTraits::CreatePlugin(config);

		auto pTransaction = CreateTransactionWithPropertiesAndName<TTraits>(1, 0);

		// Act:
		pPlugin->publish(*pTransaction, mosaicNameSub);

		// Assert:
		ASSERT_EQ(1u, mosaicNameSub.numMatchingNotifications());
		EXPECT_EQ(0u, mosaicNameSub.matchingNotifications()[0].NameSize);
		EXPECT_FALSE(!!mosaicNameSub.matchingNotifications()[0].NamePtr);
	}

	// endregion
}}
