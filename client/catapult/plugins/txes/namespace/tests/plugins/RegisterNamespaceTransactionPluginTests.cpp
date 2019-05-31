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

#include "src/plugins/RegisterNamespaceTransactionPlugin.h"
#include "src/model/NamespaceNotifications.h"
#include "src/model/RegisterNamespaceTransaction.h"
#include "catapult/utils/MemoryUtils.h"
#include "catapult/constants.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS RegisterNamespaceTransactionPluginTests

	namespace {
		DEFINE_TRANSACTION_PLUGIN_WITH_CONFIG_TEST_TRAITS(RegisterNamespace, NamespaceRentalFeeConfiguration, 2, 2,)

		constexpr UnresolvedMosaicId Currency_Mosaic_Id(1234);

		NamespaceRentalFeeConfiguration CreateRentalFeeConfiguration(Amount rootFeePerBlock, Amount childFee) {
			return {
				test::GenerateRandomByteArray<Key>(),
				Currency_Mosaic_Id,
				test::GenerateRandomUnresolvedAddress(),
				rootFeePerBlock,
				childFee,
				test::GenerateRandomByteArray<Key>()
			};
		}

		template<typename TTraits>
		auto CreateTransactionFromTraits(model::NamespaceType namespaceType) {
			auto pTransaction = std::make_unique<typename TTraits::TransactionType>();
			pTransaction->NamespaceType = namespaceType;
			test::FillWithRandomData(pTransaction->Signer);
			return pTransaction;
		}
	}

	DEFINE_BASIC_EMBEDDABLE_TRANSACTION_PLUGIN_TESTS(
			TEST_CLASS,
			,
			,
			Entity_Type_Register_Namespace,
			CreateRentalFeeConfiguration(Amount(0), Amount(0)))

	PLUGIN_TEST(CanCalculateSize) {
		// Arrange:
		auto pPlugin = TTraits::CreatePlugin(CreateRentalFeeConfiguration(Amount(0), Amount(0)));

		typename TTraits::TransactionType transaction;
		transaction.Size = 0;
		transaction.NamespaceNameSize = 100;

		// Act:
		auto realSize = pPlugin->calculateRealSize(transaction);

		// Assert:
		EXPECT_EQ(sizeof(typename TTraits::TransactionType) + 100, realSize);
	}

	PLUGIN_TEST(CanExtractAccounts) {
		// Arrange:
		mocks::MockNotificationSubscriber sub;
		auto config = CreateRentalFeeConfiguration(Amount(0), Amount(0));
		auto pPlugin = TTraits::CreatePlugin(config);

		typename TTraits::TransactionType transaction;
		transaction.Duration = BlockDuration(1);
		test::FillWithRandomData(transaction.Signer);

		// Act:
		test::PublishTransaction(*pPlugin, transaction, sub);

		// Assert:
		EXPECT_EQ(6u, sub.numNotifications());
		EXPECT_EQ(0u, sub.numAddresses());
		EXPECT_EQ(1u, sub.numKeys());

		EXPECT_TRUE(sub.contains(config.SinkPublicKey));
	}

	// region balance change

	namespace {
		template<typename TTraits, typename TAssertTransfers>
		void RunBalanceChangeObserverTest(
				typename TTraits::TransactionType& transaction,
				bool isSignerExempt,
				TAssertTransfers assertTransfers) {
			// Arrange:
			mocks::MockNotificationSubscriber sub;
			auto config = CreateRentalFeeConfiguration(Amount(987), Amount(777));
			auto pPlugin = TTraits::CreatePlugin(config);

			// - prepare the transaction
			if (isSignerExempt)
				transaction.Signer = config.NemesisPublicKey;

			// Act:
			test::PublishTransaction(*pPlugin, transaction, sub);

			// Assert:
			assertTransfers(sub, transaction.Signer, config.SinkAddress);
		}
	}

	PLUGIN_TEST(RentalFeeIsExtractedFromNonNemesisRootNamespaceWithFiniteLease) {
		// Arrange:
		auto pTransaction = CreateTransactionFromTraits<TTraits>(model::NamespaceType::Root);
		pTransaction->Duration = BlockDuration(123);

		// Act:
		RunBalanceChangeObserverTest<TTraits>(*pTransaction, false, [](const auto& sub, const auto& signer, const auto& recipient) {
			// Assert:
			EXPECT_EQ(6u, sub.numNotifications());
			EXPECT_EQ(1u, sub.numTransfers());
			EXPECT_TRUE(sub.contains(signer, recipient, Currency_Mosaic_Id, Amount(987 * 123)));
		});
	}

	PLUGIN_TEST(RentalFeeIsExemptedFromNemesisRootNamespaceWithFiniteLease) {
		// Arrange:
		auto pTransaction = CreateTransactionFromTraits<TTraits>(model::NamespaceType::Root);
		pTransaction->Duration = BlockDuration(123);

		// Act:
		RunBalanceChangeObserverTest<TTraits>(*pTransaction, true, [](const auto& sub, const auto&, const auto&) {
			// Assert:
			EXPECT_EQ(4u, sub.numNotifications());
			EXPECT_EQ(0u, sub.numTransfers());
		});
	}

	PLUGIN_TEST(RentalFeeIsExemptedFromNonNemesisRootNamespaceWithEternalLease) {
		// Arrange:
		auto pTransaction = CreateTransactionFromTraits<TTraits>(model::NamespaceType::Root);
		pTransaction->Duration = Eternal_Artifact_Duration;

		// Act:
		RunBalanceChangeObserverTest<TTraits>(*pTransaction, false, [](const auto& sub, const auto&, const auto&) {
			// Assert:
			EXPECT_EQ(4u, sub.numNotifications());
			EXPECT_EQ(0u, sub.numTransfers());
		});
	}

	PLUGIN_TEST(RentalFeeIsExemptedFromNemesisRootNamespaceWithEternalLease) {
		// Arrange:
		auto pTransaction = CreateTransactionFromTraits<TTraits>(model::NamespaceType::Root);
		pTransaction->Duration = Eternal_Artifact_Duration;

		// Act:
		RunBalanceChangeObserverTest<TTraits>(*pTransaction, true, [](const auto& sub, const auto&, const auto&) {
			// Assert:
			EXPECT_EQ(4u, sub.numNotifications());
			EXPECT_EQ(0u, sub.numTransfers());
		});
	}

	PLUGIN_TEST(RentalFeeIsExtractedFromNonNemesisChildNamespace) {
		// Arrange:
		auto pTransaction = CreateTransactionFromTraits<TTraits>(model::NamespaceType::Child);

		// Act:
		RunBalanceChangeObserverTest<TTraits>(*pTransaction, false, [](const auto& sub, const auto& signer, const auto& recipient) {
			// Assert:
			EXPECT_EQ(6u, sub.numNotifications());
			EXPECT_EQ(1u, sub.numTransfers());
			EXPECT_TRUE(sub.contains(signer, recipient, Currency_Mosaic_Id, Amount(777)));
		});
	}

	PLUGIN_TEST(RentalFeeIsExemptedFromNemesisChildNamespace) {
		// Arrange:
		auto pTransaction = CreateTransactionFromTraits<TTraits>(model::NamespaceType::Child);

		// Act:
		RunBalanceChangeObserverTest<TTraits>(*pTransaction, true, [](const auto& sub, const auto&, const auto&) {
			// Assert:
			EXPECT_EQ(4u, sub.numNotifications());
			EXPECT_EQ(0u, sub.numTransfers());
		});
	}

	// endregion

	// region registration

	namespace {
		constexpr Amount Default_Root_Rental_Fee_Per_Block(123);
		constexpr Amount Default_Child_Rental_Fee(543);

		template<typename TTraits>
		auto CreateTransactionWithName(uint8_t nameSize) {
			using TransactionType = typename TTraits::TransactionType;
			uint32_t entitySize = sizeof(TransactionType) + nameSize;
			auto pTransaction = utils::MakeUniqueWithSize<TransactionType>(entitySize);
			pTransaction->Size = entitySize;
			pTransaction->NamespaceNameSize = nameSize;
			test::FillWithRandomData(pTransaction->Signer);
			return pTransaction;
		}

		template<typename TTraits>
		struct RegisterNamespaceTransactionPluginTestContext {
		public:
			using TransactionType = typename TTraits::TransactionType;

		public:
			template<typename TTransactionPlugin>
			void PublishTransaction(const TTransactionPlugin& plugin, const TransactionType& transaction) {
				test::PublishTransaction(plugin, transaction, NamespaceSub);
				test::PublishTransaction(plugin, transaction, NameSub);
				test::PublishTransaction(plugin, transaction, RootSub);
				test::PublishTransaction(plugin, transaction, ChildSub);
				test::PublishTransaction(plugin, transaction, RentalFeeSub);
			}

			void AssertNamespaceNotification(NamespaceType namespaceType) {
				ASSERT_EQ(1u, NamespaceSub.numMatchingNotifications());
				EXPECT_EQ(namespaceType, NamespaceSub.matchingNotifications()[0].NamespaceType);
			}

			void AssertNamespaceNameNotification(const uint8_t* namespaceName, NamespaceId parentId) {
				ASSERT_EQ(1u, NameSub.numMatchingNotifications());
				const auto& notification = NameSub.matchingNotifications()[0];
				EXPECT_EQ(NamespaceId(768), notification.NamespaceId);
				EXPECT_EQ(parentId, notification.ParentId);
				EXPECT_EQ(12u, notification.NameSize);
				EXPECT_EQ(namespaceName, notification.NamePtr);
			}

			void AssertNamespaceRootNotification(const Key& signer) {
				ASSERT_EQ(1u, RootSub.numMatchingNotifications());
				const auto& notification = RootSub.matchingNotifications()[0];
				EXPECT_EQ(signer, notification.Signer);
				EXPECT_EQ(NamespaceId(768), notification.NamespaceId);
				EXPECT_EQ(BlockDuration(1234), notification.Duration);
			}

			void AssertNamespaceChildNotification(const Key& signer) {
				ASSERT_EQ(1u, ChildSub.numMatchingNotifications());
				const auto& notification = ChildSub.matchingNotifications()[0];
				EXPECT_EQ(signer, notification.Signer);
				EXPECT_EQ(NamespaceId(768), notification.NamespaceId);
				EXPECT_EQ(NamespaceId(123), notification.ParentId);
			}

			void AssertNamespaceRentalFeeNotification(const Key& signer, const UnresolvedAddress& recipientSink, Amount expectedFee) {
				ASSERT_EQ(1u, RentalFeeSub.numMatchingNotifications());
				const auto& notification = RentalFeeSub.matchingNotifications()[0];
				EXPECT_EQ(signer, notification.Sender);
				EXPECT_EQ(recipientSink, notification.Recipient);
				EXPECT_EQ(Currency_Mosaic_Id, notification.MosaicId);
				EXPECT_EQ(expectedFee, notification.Amount);
			}

		public:
			mocks::MockTypedNotificationSubscriber<NamespaceNotification> NamespaceSub;
			mocks::MockTypedNotificationSubscriber<NamespaceNameNotification> NameSub;
			mocks::MockTypedNotificationSubscriber<RootNamespaceNotification> RootSub;
			mocks::MockTypedNotificationSubscriber<ChildNamespaceNotification> ChildSub;
			mocks::MockTypedNotificationSubscriber<NamespaceRentalFeeNotification> RentalFeeSub;
		};
	}

	PLUGIN_TEST(CanExtractRegistrationNotificationsFromRootRegistration) {
		// Arrange:
		RegisterNamespaceTransactionPluginTestContext<TTraits> testContext;
		auto config = CreateRentalFeeConfiguration(Default_Root_Rental_Fee_Per_Block, Default_Child_Rental_Fee);
		auto pPlugin = TTraits::CreatePlugin(config);

		auto pTransaction = CreateTransactionWithName<TTraits>(12);
		pTransaction->NamespaceType = NamespaceType::Root;
		pTransaction->NamespaceId = NamespaceId(768);
		pTransaction->Duration = BlockDuration(1234);

		// Act:
		const auto& transaction = *pTransaction;
		testContext.PublishTransaction(*pPlugin, transaction);

		// Assert:
		testContext.AssertNamespaceNotification(NamespaceType::Root);
		testContext.AssertNamespaceNameNotification(transaction.NamePtr(), Namespace_Base_Id);
		testContext.AssertNamespaceRootNotification(transaction.Signer);
		ASSERT_EQ(0u, testContext.ChildSub.numMatchingNotifications());
		auto expectedFee = Amount(Default_Root_Rental_Fee_Per_Block.unwrap() * 1234);
		testContext.AssertNamespaceRentalFeeNotification(transaction.Signer, config.SinkAddress, expectedFee);
	}

	namespace {
		template<typename TTraits>
		void AssertCanExtractRegistrationNotificationsFromChildRegistration(NamespaceType namespaceType) {
			// Arrange:
			RegisterNamespaceTransactionPluginTestContext<TTraits> testContext;
			auto config = CreateRentalFeeConfiguration(Default_Root_Rental_Fee_Per_Block, Default_Child_Rental_Fee);
			auto pPlugin = TTraits::CreatePlugin(config);

			auto pTransaction = CreateTransactionWithName<TTraits>(12);
			pTransaction->NamespaceType = namespaceType;
			pTransaction->NamespaceId = NamespaceId(768);
			pTransaction->ParentId = NamespaceId(123);

			// Act:
			testContext.PublishTransaction(*pPlugin, *pTransaction);

			// Assert:
			const auto& transaction = *pTransaction;
			testContext.AssertNamespaceNotification(namespaceType);
			testContext.AssertNamespaceNameNotification(transaction.NamePtr(), NamespaceId(123));
			ASSERT_EQ(0u, testContext.RootSub.numMatchingNotifications());
			testContext.AssertNamespaceChildNotification(transaction.Signer);
			testContext.AssertNamespaceRentalFeeNotification(transaction.Signer, config.SinkAddress, Default_Child_Rental_Fee);
		}
	}

	PLUGIN_TEST(CanExtractRegistrationNotificationsFromChildRegistration) {
		// Assert:
		AssertCanExtractRegistrationNotificationsFromChildRegistration<TTraits>(NamespaceType::Child);
	}

	PLUGIN_TEST(CanExtractRegistrationNotificationsFromOtherRegistration) {
		// Assert: even though namespace type is unknown, all notifications should still be propagated
		AssertCanExtractRegistrationNotificationsFromChildRegistration<TTraits>(static_cast<NamespaceType>(3));
	}

	PLUGIN_TEST(CanExtractNamespaceNameNotificationWhenThereIsNoName) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<NamespaceNameNotification> nsNameSub;
		auto config = CreateRentalFeeConfiguration(Amount(0), Amount(0));
		auto pPlugin = TTraits::CreatePlugin(config);

		auto pTransaction = CreateTransactionWithName<TTraits>(0);

		// Act:
		test::PublishTransaction(*pPlugin, *pTransaction, nsNameSub);

		// Assert:
		ASSERT_EQ(1u, nsNameSub.numMatchingNotifications());
		EXPECT_EQ(0u, nsNameSub.matchingNotifications()[0].NameSize);
		EXPECT_FALSE(!!nsNameSub.matchingNotifications()[0].NamePtr);
	}

	// endregion
}}
