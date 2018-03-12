#include "src/plugins/RegisterNamespaceTransactionPlugin.h"
#include "src/model/NamespaceNotifications.h"
#include "src/model/RegisterNamespaceTransaction.h"
#include "catapult/utils/MemoryUtils.h"
#include "catapult/constants.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/plugins/TransactionPluginTestUtils.h"
#include "tests/TestHarness.h"

using namespace catapult::model;

namespace catapult { namespace plugins {

#define TEST_CLASS RegisterNamespaceTransactionPluginTests

	namespace {
		TRANSACTION_PLUGIN_WITH_CONFIG_TEST_TRAITS(RegisterNamespace, NamespaceRentalFeeConfiguration);

		NamespaceRentalFeeConfiguration CreateRentalFeeConfiguration(Amount rootFeePerBlock, Amount childFee) {
			return {
				test::GenerateRandomData<Key_Size>(),
				test::GenerateRandomData<Address_Decoded_Size>(),
				rootFeePerBlock,
				childFee,
				test::GenerateRandomData<Key_Size>()
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
			pPlugin->publish(transaction, sub);

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
			EXPECT_EQ(5u, sub.numNotifications());
			EXPECT_EQ(1u, sub.numTransfers());
			EXPECT_TRUE(sub.contains(signer, recipient, Xem_Id, Amount(987 * 123)));
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
			EXPECT_EQ(5u, sub.numNotifications());
			EXPECT_EQ(1u, sub.numTransfers());
			EXPECT_TRUE(sub.contains(signer, recipient, Xem_Id, Amount(777)));
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

	PLUGIN_TEST(CanExtractRegistrationNotificationsFromRootRegistration) {
		// Arrange:
		mocks::MockTypedNotificationSubscriber<NamespaceNameNotification> nsNameSub;
		mocks::MockTypedNotificationSubscriber<NamespaceNotification> nsSub;
		mocks::MockTypedNotificationSubscriber<RootNamespaceNotification> nsRootSub;
		mocks::MockTypedNotificationSubscriber<ChildNamespaceNotification> nsChildSub;
		auto config = CreateRentalFeeConfiguration(Amount(0), Amount(0));
		auto pPlugin = TTraits::CreatePlugin(config);

		auto pTransaction = CreateTransactionWithName<TTraits>(12);
		pTransaction->NamespaceType = NamespaceType::Root;
		pTransaction->NamespaceId = NamespaceId(768);
		pTransaction->Duration = BlockDuration(123);

		// Act:
		pPlugin->publish(*pTransaction, nsNameSub);
		pPlugin->publish(*pTransaction, nsSub);
		pPlugin->publish(*pTransaction, nsRootSub);
		pPlugin->publish(*pTransaction, nsChildSub);

		// Assert:
		ASSERT_EQ(1u, nsNameSub.numMatchingNotifications());
		EXPECT_EQ(NamespaceId(768), nsNameSub.matchingNotifications()[0].NamespaceId);
		EXPECT_EQ(Namespace_Base_Id, nsNameSub.matchingNotifications()[0].ParentId);
		EXPECT_EQ(12u, nsNameSub.matchingNotifications()[0].NameSize);
		EXPECT_EQ(pTransaction->NamePtr(), nsNameSub.matchingNotifications()[0].NamePtr);

		ASSERT_EQ(1u, nsSub.numMatchingNotifications());
		EXPECT_EQ(NamespaceType::Root, nsSub.matchingNotifications()[0].NamespaceType);

		ASSERT_EQ(1u, nsRootSub.numMatchingNotifications());
		EXPECT_EQ(pTransaction->Signer, nsRootSub.matchingNotifications()[0].Signer);
		EXPECT_EQ(NamespaceId(768), nsRootSub.matchingNotifications()[0].NamespaceId);
		EXPECT_EQ(BlockDuration(123), nsRootSub.matchingNotifications()[0].Duration);

		ASSERT_EQ(0u, nsChildSub.numMatchingNotifications());
	}

	namespace {
		template<typename TTraits>
		void AssertCanExtractRegistrationNotificationsFromChildRegistration(NamespaceType namespaceType) {
			// Arrange:
			mocks::MockTypedNotificationSubscriber<NamespaceNameNotification> nsNameSub;
			mocks::MockTypedNotificationSubscriber<NamespaceNotification> nsSub;
			mocks::MockTypedNotificationSubscriber<RootNamespaceNotification> nsRootSub;
			mocks::MockTypedNotificationSubscriber<ChildNamespaceNotification> nsChildSub;
			auto config = CreateRentalFeeConfiguration(Amount(0), Amount(0));
			auto pPlugin = TTraits::CreatePlugin(config);

			auto pTransaction = CreateTransactionWithName<TTraits>(12);
			pTransaction->NamespaceType = namespaceType;
			pTransaction->NamespaceId = NamespaceId(768);
			pTransaction->ParentId = NamespaceId(123);

			// Act:
			pPlugin->publish(*pTransaction, nsNameSub);
			pPlugin->publish(*pTransaction, nsSub);
			pPlugin->publish(*pTransaction, nsRootSub);
			pPlugin->publish(*pTransaction, nsChildSub);

			// Assert:
			ASSERT_EQ(1u, nsNameSub.numMatchingNotifications());
			EXPECT_EQ(NamespaceId(768), nsNameSub.matchingNotifications()[0].NamespaceId);
			EXPECT_EQ(NamespaceId(123), nsNameSub.matchingNotifications()[0].ParentId);
			EXPECT_EQ(12u, nsNameSub.matchingNotifications()[0].NameSize);
			EXPECT_EQ(pTransaction->NamePtr(), nsNameSub.matchingNotifications()[0].NamePtr);

			ASSERT_EQ(1u, nsSub.numMatchingNotifications());
			EXPECT_EQ(namespaceType, nsSub.matchingNotifications()[0].NamespaceType);

			ASSERT_EQ(0u, nsRootSub.numMatchingNotifications());

			ASSERT_EQ(1u, nsChildSub.numMatchingNotifications());
			EXPECT_EQ(pTransaction->Signer, nsChildSub.matchingNotifications()[0].Signer);
			EXPECT_EQ(NamespaceId(768), nsChildSub.matchingNotifications()[0].NamespaceId);
			EXPECT_EQ(NamespaceId(123), nsChildSub.matchingNotifications()[0].ParentId);
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
		pPlugin->publish(*pTransaction, nsNameSub);

		// Assert:
		ASSERT_EQ(1u, nsNameSub.numMatchingNotifications());
		EXPECT_EQ(0u, nsNameSub.matchingNotifications()[0].NameSize);
		EXPECT_FALSE(!!nsNameSub.matchingNotifications()[0].NamePtr);
	}

	// endregion
}}
