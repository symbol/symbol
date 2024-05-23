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

#include "catapult/constants.h"
#include "catapult/utils/MemoryUtils.h"
#include "src/model/NamespaceNotifications.h"
#include "src/model/NamespaceRegistrationTransaction.h"
#include "src/plugins/NamespaceRegistrationTransactionPlugin.h"
#include "tests/TestHarness.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"

using namespace catapult::model;

namespace catapult {
namespace plugins {

#define TEST_CLASS NamespaceRegistrationTransactionPluginTests

    // region test utils

    namespace {
        DEFINE_TRANSACTION_PLUGIN_WITH_CONFIG_TEST_TRAITS(NamespaceRegistration, NamespaceRentalFeeConfiguration, 1, 1, )

        NamespaceRentalFeeConfiguration CreateRentalFeeConfiguration(Amount rootFeePerBlock, Amount childFee)
        {
            HeightDependentAddress sinkAddress(test::GenerateRandomByteArray<Address>());
            sinkAddress.trySet(test::GenerateRandomByteArray<Address>(), Height(1111));
            sinkAddress.trySet(test::GenerateRandomByteArray<Address>(), Height(2222));

            return { UnresolvedMosaicId(1234), sinkAddress, rootFeePerBlock, childFee, test::GenerateRandomByteArray<Key>() };
        }

        template <typename TTraits>
        auto CreateTransactionWithName(uint8_t nameSize)
        {
            using TransactionType = typename TTraits::TransactionType;
            uint32_t entitySize = SizeOf32<TransactionType>() + nameSize;
            auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
            pTransaction->Size = entitySize;
            pTransaction->NameSize = nameSize;
            test::FillWithRandomData(pTransaction->SignerPublicKey);
            return pTransaction;
        }
    }

    DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(
        TEST_CLASS,
        ,
        ,
        Entity_Type_Namespace_Registration,
        CreateRentalFeeConfiguration(Amount(0), Amount(0)))

    // endregion

    // region publish - root nemesis signer

    namespace {
        auto Run_Test_Height = Height(1500);

        WeakEntityInfoT<Transaction> ToTransactionInfo(
            const NamespaceRegistrationTransaction& transaction,
            const Hash256& hash,
            const BlockHeader& blockHeader)
        {
            return { transaction, hash, blockHeader };
        }

        WeakEntityInfoT<EmbeddedTransaction> ToTransactionInfo(
            const EmbeddedNamespaceRegistrationTransaction& transaction,
            const Hash256& hash,
            const BlockHeader& blockHeader)
        {
            return { transaction, hash, blockHeader };
        }

        template <typename TBuilder, typename TTransaction>
        void RunTest(const TBuilder& builder, const TTransaction& transaction, const NamespaceRentalFeeConfiguration& config)
        {
            // Arrange:
            auto hash = test::GenerateRandomByteArray<Hash256>();

            BlockHeader blockHeader;
            blockHeader.Height = Run_Test_Height;

            // Act + Assert:
            builder.runTest(ToTransactionInfo(transaction, hash, blockHeader), config);
        }

        template <typename TTraits>
        void AddCommonRootExpectations(
            typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder& builder,
            const NamespaceRentalFeeConfiguration& config,
            const typename TTraits::TransactionType& transaction)
        {
            builder.template addExpectation<AccountAddressNotification>([&config](const auto& notification) {
                EXPECT_TRUE(notification.Address.isResolved());

                EXPECT_EQ(config.SinkAddress.get(Run_Test_Height), notification.Address.resolved());
            });
            builder.template addExpectation<NamespaceRegistrationNotification>(
                [&transaction](const auto& notification) { EXPECT_EQ(transaction.RegistrationType, notification.RegistrationType); });
            builder.template addExpectation<RootNamespaceNotification>([&transaction](const auto& notification) {
                EXPECT_EQ(GetSignerAddress(transaction), notification.Owner);
                EXPECT_EQ(transaction.Id, notification.NamespaceId);
                EXPECT_EQ(transaction.Duration, notification.Duration);
            });
            builder.template addExpectation<NamespaceNameNotification>([&transaction](const auto& notification) {
                EXPECT_EQ(transaction.Id, notification.NamespaceId);
                EXPECT_EQ(Namespace_Base_Id, notification.ParentId);
                EXPECT_EQ(transaction.NameSize, notification.NameSize);
                EXPECT_EQ(transaction.NamePtr(), notification.NamePtr);
            });
        }

        template <typename TTransaction>
        void PrepareRootNamespaceFiniteDurationTransaction(TTransaction& transaction)
        {
            test::FillWithRandomData(transaction);
            transaction.RegistrationType = NamespaceRegistrationType::Root;
            transaction.Duration = BlockDuration(test::Random() | 1);
            transaction.NameSize = 11;
        }
    }

    PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenNemesisRootRegistration)
    {
        // Arrange:
        auto config = CreateRentalFeeConfiguration(Amount(987), Amount(777));

        auto pTransaction = CreateTransactionWithName<TTraits>(11);
        PrepareRootNamespaceFiniteDurationTransaction(*pTransaction);
        pTransaction->SignerPublicKey = config.NemesisSignerPublicKey;

        // Act + Assert:
        test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(
            *pTransaction,
            { AccountAddressNotification::Notification_Type,
                NamespaceRegistrationNotification::Notification_Type,
                RootNamespaceNotification::Notification_Type,
                NamespaceNameNotification::Notification_Type },
            config);
    }

    PLUGIN_TEST(CanPublishAllNotificationsWhenNemesisRootRegistration)
    {
        // Arrange:
        auto config = CreateRentalFeeConfiguration(Amount(987), Amount(777));

        auto pTransaction = CreateTransactionWithName<TTraits>(11);
        PrepareRootNamespaceFiniteDurationTransaction(*pTransaction);
        pTransaction->SignerPublicKey = config.NemesisSignerPublicKey;

        typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
        AddCommonRootExpectations<TTraits>(builder, config, *pTransaction);

        // Act + Assert:
        RunTest(builder, *pTransaction, config);
    }

    // endregion

    // region publish - root nemesis not signer (finite duration)

    PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenNotNemesisRootRegistrationWithFiniteDuration)
    {
        // Arrange:
        auto config = CreateRentalFeeConfiguration(Amount(987), Amount(777));

        auto pTransaction = CreateTransactionWithName<TTraits>(11);
        PrepareRootNamespaceFiniteDurationTransaction(*pTransaction);

        // Act + Assert:
        test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(
            *pTransaction,
            { AccountAddressNotification::Notification_Type,
                BalanceTransferNotification::Notification_Type,
                NamespaceRentalFeeNotification::Notification_Type,
                NamespaceRegistrationNotification::Notification_Type,
                RootNamespaceNotification::Notification_Type,
                NamespaceNameNotification::Notification_Type },
            config);
    }

    PLUGIN_TEST(CanPublishAllNotificationsWhenNotNemesisRootRegistrationWithFiniteDuration)
    {
        // Arrange:
        auto config = CreateRentalFeeConfiguration(Amount(987), Amount(777));

        auto pTransaction = CreateTransactionWithName<TTraits>(11);
        PrepareRootNamespaceFiniteDurationTransaction(*pTransaction);

        const auto& transaction = *pTransaction;
        typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
        AddCommonRootExpectations<TTraits>(builder, config, transaction);
        builder.template addExpectation<BalanceTransferNotification>([&config, &transaction](const auto& notification) {
            EXPECT_TRUE(notification.Sender.isResolved());
            EXPECT_TRUE(notification.Recipient.isResolved());

            EXPECT_EQ(GetSignerAddress(transaction), notification.Sender.resolved());
            EXPECT_EQ(config.SinkAddress.get(Run_Test_Height), notification.Recipient.resolved());
            EXPECT_EQ(config.CurrencyMosaicId, notification.MosaicId);
            EXPECT_EQ(Amount(987 * transaction.Duration.unwrap()), notification.Amount);
            EXPECT_EQ(BalanceTransferNotification::AmountType::Dynamic, notification.TransferAmountType);
        });
        builder.template addExpectation<NamespaceRentalFeeNotification>([&config, &transaction](const auto& notification) {
            EXPECT_TRUE(notification.Sender.isResolved());
            EXPECT_TRUE(notification.Recipient.isResolved());

            EXPECT_EQ(GetSignerAddress(transaction), notification.Sender.resolved());
            EXPECT_EQ(config.SinkAddress.get(Run_Test_Height), notification.Recipient.resolved());
            EXPECT_EQ(config.CurrencyMosaicId, notification.MosaicId);
            EXPECT_EQ(Amount(987 * transaction.Duration.unwrap()), notification.Amount);
        });

        // Act + Assert:
        RunTest(builder, transaction, config);
    }

    // endregion

    // region publish - root nemesis not signer (eternal duration)

    PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenNotNemesisRootRegistrationWithEternalDuration)
    {
        // Arrange:
        auto config = CreateRentalFeeConfiguration(Amount(987), Amount(777));

        auto pTransaction = CreateTransactionWithName<TTraits>(11);
        PrepareRootNamespaceFiniteDurationTransaction(*pTransaction);
        pTransaction->Duration = BlockDuration();

        // Act + Assert:
        test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(
            *pTransaction,
            { AccountAddressNotification::Notification_Type,
                NamespaceRegistrationNotification::Notification_Type,
                RootNamespaceNotification::Notification_Type,
                NamespaceNameNotification::Notification_Type },
            config);
    }

    PLUGIN_TEST(CanPublishAllNotificationsWhenNotNemesisRootRegistrationWithEternalDuration)
    {
        // Arrange:
        auto config = CreateRentalFeeConfiguration(Amount(987), Amount(777));

        auto pTransaction = CreateTransactionWithName<TTraits>(11);
        PrepareRootNamespaceFiniteDurationTransaction(*pTransaction);
        pTransaction->Duration = BlockDuration();

        typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
        AddCommonRootExpectations<TTraits>(builder, config, *pTransaction);

        // Act + Assert:
        RunTest(builder, *pTransaction, config);
    }

    // endregion

    // region publish - child nemesis signer

    namespace {
        template <typename TTraits>
        void AddCommonChildExpectations(
            typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder& builder,
            const NamespaceRentalFeeConfiguration& config,
            const typename TTraits::TransactionType& transaction)
        {
            builder.template addExpectation<AccountAddressNotification>([&config](const auto& notification) {
                EXPECT_TRUE(notification.Address.isResolved());

                EXPECT_EQ(config.SinkAddress.get(Run_Test_Height), notification.Address.resolved());
            });
            builder.template addExpectation<NamespaceRegistrationNotification>(
                [&transaction](const auto& notification) { EXPECT_EQ(transaction.RegistrationType, notification.RegistrationType); });
            builder.template addExpectation<ChildNamespaceNotification>([&transaction](const auto& notification) {
                EXPECT_EQ(GetSignerAddress(transaction), notification.Owner);
                EXPECT_EQ(transaction.Id, notification.NamespaceId);
                EXPECT_EQ(transaction.ParentId, notification.ParentId);
            });
            builder.template addExpectation<NamespaceNameNotification>([&transaction](const auto& notification) {
                EXPECT_EQ(transaction.Id, notification.NamespaceId);
                EXPECT_EQ(transaction.ParentId, notification.ParentId);
                EXPECT_EQ(transaction.NameSize, notification.NameSize);
                EXPECT_EQ(transaction.NamePtr(), notification.NamePtr);
            });
        }

        template <typename TTransaction>
        void PrepareChildNamespaceTransaction(TTransaction& transaction)
        {
            test::FillWithRandomData(transaction);
            transaction.RegistrationType = NamespaceRegistrationType::Child;
            transaction.ParentId = NamespaceId(test::Random() | 1);
            transaction.NameSize = 11;
        }
    }

    PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenNemesisChildRegistration)
    {
        // Arrange:
        auto config = CreateRentalFeeConfiguration(Amount(987), Amount(777));

        auto pTransaction = CreateTransactionWithName<TTraits>(11);
        PrepareChildNamespaceTransaction(*pTransaction);
        pTransaction->SignerPublicKey = config.NemesisSignerPublicKey;

        // Act + Assert:
        test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(
            *pTransaction,
            { AccountAddressNotification::Notification_Type,
                NamespaceRegistrationNotification::Notification_Type,
                ChildNamespaceNotification::Notification_Type,
                NamespaceNameNotification::Notification_Type },
            config);
    }

    PLUGIN_TEST(CanPublishAllNotificationsWhenNemesisChildRegistration)
    {
        // Arrange:
        auto config = CreateRentalFeeConfiguration(Amount(987), Amount(777));

        auto pTransaction = CreateTransactionWithName<TTraits>(11);
        PrepareChildNamespaceTransaction(*pTransaction);
        pTransaction->SignerPublicKey = config.NemesisSignerPublicKey;

        typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
        AddCommonChildExpectations<TTraits>(builder, config, *pTransaction);

        // Act + Assert:
        RunTest(builder, *pTransaction, config);
    }

    // endregion

    // region publish - child nemesis not signer

    PLUGIN_TEST(CanPublishAllNotificationsInCorrectOrderWhenNotNemesisChildRegistrationWithFiniteDuration)
    {
        // Arrange:
        auto config = CreateRentalFeeConfiguration(Amount(987), Amount(777));

        auto pTransaction = CreateTransactionWithName<TTraits>(11);
        PrepareChildNamespaceTransaction(*pTransaction);

        // Act + Assert:
        test::TransactionPluginTestUtils<TTraits>::AssertNotificationTypes(
            *pTransaction,
            { AccountAddressNotification::Notification_Type,
                BalanceTransferNotification::Notification_Type,
                NamespaceRentalFeeNotification::Notification_Type,
                NamespaceRegistrationNotification::Notification_Type,
                ChildNamespaceNotification::Notification_Type,
                NamespaceNameNotification::Notification_Type },
            config);
    }

    PLUGIN_TEST(CanPublishAllNotificationsWhenNotNemesisChildRegistrationWithFiniteDuration)
    {
        // Arrange:
        auto config = CreateRentalFeeConfiguration(Amount(987), Amount(777));

        auto pTransaction = CreateTransactionWithName<TTraits>(11);
        PrepareChildNamespaceTransaction(*pTransaction);

        const auto& transaction = *pTransaction;
        typename test::TransactionPluginTestUtils<TTraits>::PublishTestBuilder builder;
        AddCommonChildExpectations<TTraits>(builder, config, transaction);
        builder.template addExpectation<BalanceTransferNotification>([&config, &transaction](const auto& notification) {
            EXPECT_TRUE(notification.Sender.isResolved());
            EXPECT_TRUE(notification.Recipient.isResolved());

            EXPECT_EQ(GetSignerAddress(transaction), notification.Sender.resolved());
            EXPECT_EQ(config.SinkAddress.get(Run_Test_Height), notification.Recipient.resolved());
            EXPECT_EQ(config.CurrencyMosaicId, notification.MosaicId);
            EXPECT_EQ(Amount(777), notification.Amount);
            EXPECT_EQ(BalanceTransferNotification::AmountType::Dynamic, notification.TransferAmountType);
        });
        builder.template addExpectation<NamespaceRentalFeeNotification>([&config, &transaction](const auto& notification) {
            EXPECT_TRUE(notification.Sender.isResolved());
            EXPECT_TRUE(notification.Recipient.isResolved());

            EXPECT_EQ(GetSignerAddress(transaction), notification.Sender.resolved());
            EXPECT_EQ(config.SinkAddress.get(Run_Test_Height), notification.Recipient.resolved());
            EXPECT_EQ(config.CurrencyMosaicId, notification.MosaicId);
            EXPECT_EQ(Amount(777), notification.Amount);
        });

        // Act + Assert:
        RunTest(builder, transaction, config);
    }

    // endregion
}
}
