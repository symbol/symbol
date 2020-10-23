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

#include "catapult/model/NotificationPublisher.h"
#include "catapult/model/Address.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/nodeps/NumericTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS NotificationPublisherTests

	namespace {
		constexpr auto Currency_Mosaic_Id = UnresolvedMosaicId(1234);

		constexpr auto Plugin_Option_Flags = static_cast<mocks::PluginOptionFlags>(
				utils::to_underlying_type(mocks::PluginOptionFlags::Custom_Buffers)
				| utils::to_underlying_type(mocks::PluginOptionFlags::Publish_Custom_Notifications)
				| utils::to_underlying_type(mocks::PluginOptionFlags::Contains_Embeddings));

		template<typename TEntity, typename TAssertSubFunc>
		void PublishAll(const TEntity& entity, PublicationMode mode, TAssertSubFunc assertSub) {
			// Arrange:
			mocks::MockNotificationSubscriber sub;

			auto registry = mocks::CreateDefaultTransactionRegistry(Plugin_Option_Flags);
			auto pPub = CreateNotificationPublisher(registry, Currency_Mosaic_Id, mode);

			// Act:
			auto hash = test::GenerateRandomByteArray<Hash256>();
			pPub->publish(WeakEntityInfo(entity, hash), sub);

			// Assert:
			assertSub(sub);
		}

		template<typename TEntity, typename TAssertSubFunc>
		void PublishAll(const TEntity& entity, TAssertSubFunc assertSub) {
			PublishAll(entity, PublicationMode::All, assertSub);
		}

		template<typename TNotification, typename TAssertNotification>
		void PublishOne(const WeakEntityInfo& entityInfo, TAssertNotification assertNotification) {
			// Arrange:
			mocks::MockTypedNotificationSubscriber<TNotification> sub;

			auto registry = mocks::CreateDefaultTransactionRegistry(Plugin_Option_Flags);
			auto pPub = CreateNotificationPublisher(registry, Currency_Mosaic_Id);

			// Act:
			pPub->publish(entityInfo, sub);

			// Assert:
			ASSERT_EQ(1u, sub.numMatchingNotifications());
			assertNotification(sub.matchingNotifications()[0]);
		}

		template<typename TNotification, typename TEntity, typename TAssertNotification>
		void PublishOne(const TEntity& entity, const Hash256& hash, TAssertNotification assertNotification) {
			PublishOne<TNotification>(WeakEntityInfo(entity, hash), assertNotification);
		}

		template<typename TNotification, typename TEntity, typename TAssertNotification>
		void PublishOne(const TEntity& entity, TAssertNotification assertNotification) {
			PublishOne<TNotification>(entity, test::GenerateRandomByteArray<Hash256>(), assertNotification);
		}
	}

	// region block

	TEST(TEST_CLASS, CanRaiseBlockSourceChangeNotifications) {
		// Arrange:
		auto pBlock = test::GenerateEmptyRandomBlock();

		// Act:
		PublishOne<SourceChangeNotification>(*pBlock, [](const auto& notification) {
			// Assert:
			EXPECT_EQ(0u, notification.PrimaryId);
			EXPECT_EQ(0u, notification.SecondaryId);
			EXPECT_EQ(SourceChangeNotification::SourceChangeType::Absolute, notification.PrimaryChangeType);
			EXPECT_EQ(SourceChangeNotification::SourceChangeType::Absolute, notification.SecondaryChangeType);
		});
	}

	TEST(TEST_CLASS, CanRaiseBlockAccountNotifications_WithBeneficiary) {
		// Arrange:
		auto pBlock = test::GenerateEmptyRandomBlock();
		test::FillWithRandomData(pBlock->SignerPublicKey);
		test::FillWithRandomData(pBlock->BeneficiaryAddress);

		// Act:
		PublishAll(*pBlock, [&block = *pBlock](const auto& sub) {
			// Assert:
			EXPECT_EQ(7u, sub.numNotifications());
			EXPECT_EQ(1u, sub.numAddresses());
			EXPECT_EQ(1u, sub.numKeys());

			EXPECT_TRUE(sub.contains(block.SignerPublicKey));
			EXPECT_TRUE(sub.contains(block.BeneficiaryAddress));
		});
	}

	TEST(TEST_CLASS, CanRaiseBlockAccountNotifications_WithoutBeneficiary) {
		// Arrange:
		auto pBlock = test::GenerateEmptyRandomBlock();
		test::FillWithRandomData(pBlock->SignerPublicKey);
		pBlock->BeneficiaryAddress = GetSignerAddress(*pBlock);

		// Act:
		PublishAll(*pBlock, [&block = *pBlock](const auto& sub) {
			// Assert:
			EXPECT_EQ(6u, sub.numNotifications());
			EXPECT_EQ(0u, sub.numAddresses());
			EXPECT_EQ(1u, sub.numKeys());

			EXPECT_TRUE(sub.contains(block.SignerPublicKey));
		});
	}

	TEST(TEST_CLASS, CanRaiseBeneficiaryAccountNotification) {
		// Arrange:
		auto pBlock = test::GenerateEmptyRandomBlock();
		test::FillWithRandomData(pBlock->SignerPublicKey);
		test::FillWithRandomData(pBlock->BeneficiaryAddress);

		// Act:
		PublishOne<AccountAddressNotification>(*pBlock, [&beneficiaryAddress = pBlock->BeneficiaryAddress](const auto& notification) {
			// Assert:
			EXPECT_TRUE(notification.Address.isResolved());

			EXPECT_EQ(beneficiaryAddress, notification.Address.resolved());
		});
	}

	TEST(TEST_CLASS, CanRaiseBlockTypeNotifications) {
		// Arrange:
		auto pBlock = test::GenerateImportanceBlockWithTransactions(0);
		pBlock->Type = Entity_Type_Block_Importance;
		pBlock->Height = Height(9876);

		// Act:
		PublishOne<BlockTypeNotification>(*pBlock, [](const auto& notification) {
			// Assert:
			EXPECT_EQ(Entity_Type_Block_Importance, notification.BlockType);
			EXPECT_EQ(Height(9876), notification.BlockHeight);
		});
	}

	TEST(TEST_CLASS, CanRaiseBlockEntityNotifications) {
		// Arrange:
		auto pBlock = test::GenerateEmptyRandomBlock();
		pBlock->Version = 0x5A;
		pBlock->Network = static_cast<NetworkIdentifier>(0x11);
		pBlock->Type = static_cast<model::EntityType>(0xAA55);

		// Act:
		PublishOne<EntityNotification>(*pBlock, [](const auto& notification) {
			// Assert:
			auto expectedVersion = Block::Current_Version;
			EXPECT_EQ(static_cast<NetworkIdentifier>(0x11), notification.NetworkIdentifier);
			EXPECT_EQ(static_cast<model::EntityType>(0xAA55), notification.EntityType);
			EXPECT_EQ(0x5Au, notification.EntityVersion);
			EXPECT_EQ(expectedVersion, notification.MinVersion);
			EXPECT_EQ(expectedVersion, notification.MaxVersion);
		});
	}

	TEST(TEST_CLASS, CanRaiseBlockSignatureNotifications_BlockNormal) {
		// Arrange:
		auto pBlock = test::GenerateEmptyRandomBlock();
		test::FillWithRandomData(pBlock->SignerPublicKey);
		test::FillWithRandomData(pBlock->Signature);

		// Act:
		PublishOne<SignatureNotification>(*pBlock, [&block = *pBlock](const auto& notification) {
			// Assert:
			EXPECT_EQ(block.SignerPublicKey, notification.SignerPublicKey);
			EXPECT_EQ(block.Signature, notification.Signature);
			EXPECT_EQ(test::AsVoidPointer(&block.Version), test::AsVoidPointer(notification.Data.pData));
			EXPECT_EQ(sizeof(BlockHeader) - VerifiableEntity::Header_Size, notification.Data.Size);
			EXPECT_EQ(SignatureNotification::ReplayProtectionMode::Disabled, notification.DataReplayProtectionMode);
		});
	}

	TEST(TEST_CLASS, CanRaiseBlockSignatureNotifications_BlockImportance) {
		// Arrange:
		auto pBlock = test::GenerateImportanceBlockWithTransactions(0);
		test::FillWithRandomData(pBlock->SignerPublicKey);
		test::FillWithRandomData(pBlock->Signature);

		// Act:
		PublishOne<SignatureNotification>(*pBlock, [&block = *pBlock](const auto& notification) {
			// Assert:
			EXPECT_EQ(block.SignerPublicKey, notification.SignerPublicKey);
			EXPECT_EQ(block.Signature, notification.Signature);
			EXPECT_EQ(test::AsVoidPointer(&block.Version), test::AsVoidPointer(notification.Data.pData));
			EXPECT_EQ(sizeof(BlockHeader) + sizeof(ImportanceBlockFooter) - VerifiableEntity::Header_Size, notification.Data.Size);
			EXPECT_EQ(SignatureNotification::ReplayProtectionMode::Disabled, notification.DataReplayProtectionMode);
		});
	}

	namespace {
		std::unique_ptr<Block> GenerateBlockWithTransactionSizes(const std::vector<uint32_t>& sizes) {
			test::ConstTransactions transactions;
			for (auto size : sizes) {
				auto pTransaction = test::GenerateRandomTransactionWithSize(size);
				pTransaction->Type = mocks::MockTransaction::Entity_Type;
				pTransaction->MaxFee = Amount(10 * size);
				transactions.push_back(std::move(pTransaction));
			}

			auto pBlock = test::GenerateBlockWithTransactions(transactions);
			test::FillWithRandomData(pBlock->SignerPublicKey);
			return pBlock;
		}
	}

	TEST(TEST_CLASS, CanRaiseBlockNotifications_BlockWithoutTransactions) {
		// Arrange:
		auto pBlock = GenerateBlockWithTransactionSizes({});
		pBlock->Timestamp = Timestamp(123);
		pBlock->Difficulty = Difficulty(575);
		pBlock->FeeMultiplier = BlockFeeMultiplier(3);

		// Act:
		PublishOne<BlockNotification>(*pBlock, [&block = *pBlock](const auto& notification) {
			// Assert:
			EXPECT_EQ(GetSignerAddress(block), notification.Harvester);
			EXPECT_EQ(block.BeneficiaryAddress, notification.Beneficiary);
			EXPECT_EQ(Timestamp(123), notification.Timestamp);
			EXPECT_EQ(Difficulty(575), notification.Difficulty);
			EXPECT_EQ(BlockFeeMultiplier(3), notification.FeeMultiplier);
			EXPECT_EQ(Amount(0), notification.TotalFee);
			EXPECT_EQ(0u, notification.NumTransactions);
		});
	}

	TEST(TEST_CLASS, CanRaiseBlockNotifications_BlockWithTransactions) {
		// Arrange:
		auto pBlock = GenerateBlockWithTransactionSizes({ 211, 225, 217 });
		pBlock->Timestamp = Timestamp(432);
		pBlock->Difficulty = Difficulty(575);
		pBlock->FeeMultiplier = BlockFeeMultiplier(3);

		// Act:
		PublishOne<BlockNotification>(*pBlock, [&block = *pBlock](const auto& notification) {
			// Assert: mock embeddedCount is `Size % 100`
			EXPECT_EQ(GetSignerAddress(block), notification.Harvester);
			EXPECT_EQ(block.BeneficiaryAddress, notification.Beneficiary);
			EXPECT_EQ(Timestamp(432), notification.Timestamp);
			EXPECT_EQ(Difficulty(575), notification.Difficulty);
			EXPECT_EQ(BlockFeeMultiplier(3), notification.FeeMultiplier);
			EXPECT_EQ(Amount(3 * 653), notification.TotalFee);
			EXPECT_EQ(3u + 11 + 25 + 17, notification.NumTransactions);
		});
	}

	TEST(TEST_CLASS, CanRaiseImportanceBlockNotifications_BlockImportance) {
		// Arrange:
		auto pBlock = test::GenerateImportanceBlockWithTransactions(0);
		auto& blockFooter = GetBlockFooter<ImportanceBlockFooter>(*pBlock);
		blockFooter.VotingEligibleAccountsCount = 432;
		blockFooter.HarvestingEligibleAccountsCount = 575;
		blockFooter.TotalVotingBalance = Amount(3);
		test::FillWithRandomData(blockFooter.PreviousImportanceBlockHash);

		// Act:
		PublishOne<ImportanceBlockNotification>(*pBlock, [&blockFooter](const auto& notification) {
			// Assert:
			EXPECT_EQ(432u, notification.VotingEligibleAccountsCount);
			EXPECT_EQ(575u, notification.HarvestingEligibleAccountsCount);
			EXPECT_EQ(Amount(3), notification.TotalVotingBalance);
			EXPECT_EQ(blockFooter.PreviousImportanceBlockHash, notification.PreviousImportanceBlockHash);
		});
	}

	TEST(TEST_CLASS, CanPublishBlockNotificationsWithModeBasic_BlockNormal) {
		// Arrange:
		auto pBlock = test::GenerateEmptyRandomBlock();

		// Act:
		PublishAll(*pBlock, PublicationMode::Basic, [](const auto& sub) {
			// Assert: no notifications were suppressed (blocks do not have custom notifications)
			ASSERT_EQ(7u, sub.numNotifications());
			EXPECT_EQ(Core_Source_Change_Notification, sub.notificationTypes()[0]);
			EXPECT_EQ(Core_Register_Account_Public_Key_Notification, sub.notificationTypes()[1]);
			EXPECT_EQ(Core_Register_Account_Address_Notification, sub.notificationTypes()[2]);
			EXPECT_EQ(Core_Block_Type_Notification, sub.notificationTypes()[3]);
			EXPECT_EQ(Core_Entity_Notification, sub.notificationTypes()[4]);
			EXPECT_EQ(Core_Block_Notification, sub.notificationTypes()[5]);
			EXPECT_EQ(Core_Signature_Notification, sub.notificationTypes()[6]);
		});
	}

	TEST(TEST_CLASS, CanPublishBlockNotificationsWithModeCustom_BlockNormal) {
		// Arrange:
		auto pBlock = test::GenerateEmptyRandomBlock();

		// Act:
		PublishAll(*pBlock, PublicationMode::Custom, [](const auto& sub) {
			// Assert: all notifications were suppressed (blocks do not have custom notifications)
			ASSERT_EQ(0u, sub.numNotifications());
		});
	}

	TEST(TEST_CLASS, CanPublishBlockNotificationsWithModeBasic_BlockImportance) {
		// Arrange:
		auto pBlock = test::GenerateImportanceBlockWithTransactions(0);

		// Act:
		PublishAll(*pBlock, PublicationMode::Basic, [](const auto& sub) {
			// Assert: no notifications were suppressed (blocks do not have custom notifications)
			ASSERT_EQ(8u, sub.numNotifications());
			EXPECT_EQ(Core_Source_Change_Notification, sub.notificationTypes()[0]);
			EXPECT_EQ(Core_Register_Account_Public_Key_Notification, sub.notificationTypes()[1]);
			EXPECT_EQ(Core_Register_Account_Address_Notification, sub.notificationTypes()[2]);
			EXPECT_EQ(Core_Block_Type_Notification, sub.notificationTypes()[3]);
			EXPECT_EQ(Core_Entity_Notification, sub.notificationTypes()[4]);
			EXPECT_EQ(Core_Block_Notification, sub.notificationTypes()[5]);
			EXPECT_EQ(Core_Block_Importance_Notification, sub.notificationTypes()[6]);
			EXPECT_EQ(Core_Signature_Notification, sub.notificationTypes()[7]);
		});
	}

	TEST(TEST_CLASS, CanPublishBlockNotificationsWithModeCustom_BlockImportance) {
		// Arrange:
		auto pBlock = test::GenerateImportanceBlockWithTransactions(0);

		// Act:
		PublishAll(*pBlock, PublicationMode::Custom, [](const auto& sub) {
			// Assert: all notifications were suppressed (blocks do not have custom notifications)
			ASSERT_EQ(0u, sub.numNotifications());
		});
	}

	// endregion

	// region transaction

	TEST(TEST_CLASS, CanRaiseTransactionSourceChangeNotifications) {
		// Arrange:
		auto pTransaction = mocks::CreateMockTransaction(0);

		// Act:
		PublishOne<SourceChangeNotification>(*pTransaction, [](const auto& notification) {
			// Assert:
			EXPECT_EQ(1u, notification.PrimaryId);
			EXPECT_EQ(0u, notification.SecondaryId);
			EXPECT_EQ(SourceChangeNotification::SourceChangeType::Relative, notification.PrimaryChangeType);
			EXPECT_EQ(SourceChangeNotification::SourceChangeType::Absolute, notification.SecondaryChangeType);
		});
	}

	TEST(TEST_CLASS, CanRaiseTransactionAccountNotifications) {
		// Arrange:
		auto pTransaction = mocks::CreateMockTransaction(0);
		test::FillWithRandomData(pTransaction->SignerPublicKey);
		test::FillWithRandomData(pTransaction->RecipientPublicKey);

		// Act:
		PublishAll(*pTransaction, [&transaction = *pTransaction](const auto& sub) {
			// Assert: both signer (from notification publisher) and recipient (from custom publish implementation) are raised
			EXPECT_EQ(0u, sub.numAddresses());
			EXPECT_EQ(2u, sub.numKeys());

			EXPECT_TRUE(sub.contains(transaction.SignerPublicKey));
			EXPECT_TRUE(sub.contains(transaction.RecipientPublicKey));
		});
	}

	TEST(TEST_CLASS, CanRaiseTransactionEntityNotifications) {
		// Arrange:
		auto pTransaction = mocks::CreateMockTransaction(0);
		pTransaction->Version = 0x5A;
		pTransaction->Network = static_cast<NetworkIdentifier>(0x11);

		// Act:
		PublishOne<EntityNotification>(*pTransaction, [](const auto& notification) {
			// Assert:
			EXPECT_EQ(static_cast<NetworkIdentifier>(0x11), notification.NetworkIdentifier);
			EXPECT_EQ(mocks::MockTransaction::Entity_Type, notification.EntityType);
			EXPECT_EQ(0x5Au, notification.EntityVersion);
			EXPECT_EQ(0x02u, notification.MinVersion); // from MockTransaction
			EXPECT_EQ(0xFEu, notification.MaxVersion);
		});
	}

	TEST(TEST_CLASS, CanRaiseTransactionSignatureNotifications) {
		// Arrange:
		auto pTransaction = mocks::CreateMockTransaction(12);
		test::FillWithRandomData(pTransaction->SignerPublicKey);
		test::FillWithRandomData(pTransaction->Signature);

		// Act:
		PublishOne<SignatureNotification>(*pTransaction, [&transaction = *pTransaction](const auto& notification) {
			// Assert:
			EXPECT_EQ(transaction.SignerPublicKey, notification.SignerPublicKey);
			EXPECT_EQ(transaction.Signature, notification.Signature);

			// - notice that mock plugin is configured with PluginOptionFlags::Custom_Buffers so dataBuffer() contains only data payload
			EXPECT_EQ(test::AsVoidPointer(&transaction + 1), test::AsVoidPointer(notification.Data.pData));
			EXPECT_EQ(12u, notification.Data.Size);
			EXPECT_EQ(SignatureNotification::ReplayProtectionMode::Enabled, notification.DataReplayProtectionMode);
		});
	}

	TEST(TEST_CLASS, CanRaiseTransactionNotifications) {
		// Arrange:
		auto hash = test::GenerateRandomByteArray<Hash256>();
		auto pTransaction = mocks::CreateMockTransaction(12);
		test::FillWithRandomData(pTransaction->SignerPublicKey);
		pTransaction->Deadline = Timestamp(454);

		// Act:
		PublishOne<TransactionNotification>(*pTransaction, hash, [&transaction = *pTransaction, &hash](const auto& notification) {
			// Assert:
			EXPECT_EQ(GetSignerAddress(transaction), notification.Sender);
			EXPECT_EQ(hash, notification.TransactionHash);
			EXPECT_EQ(static_cast<EntityType>(mocks::MockTransaction::Entity_Type), notification.TransactionType);
			EXPECT_EQ(Timestamp(454), notification.Deadline);
		});
	}

	TEST(TEST_CLASS, CanRaiseTransactionDeadlineNotifications) {
		// Arrange:
		auto pTransaction = mocks::CreateMockTransaction(12);
		pTransaction->Deadline = Timestamp(454);

		// Act:
		PublishOne<TransactionDeadlineNotification>(*pTransaction, [](const auto& notification) {
			// Assert:
			EXPECT_EQ(Timestamp(454), notification.Deadline);
			EXPECT_EQ(utils::TimeSpan::FromMilliseconds(0xEEEE'EEEE'EEEE'1234), notification.MaxLifetime); // from MockTransaction
		});
	}

	TEST(TEST_CLASS, CanRaiseTransactionFeeNotification_BlockIndependent) {
		// Arrange:
		auto pTransaction = mocks::CreateMockTransaction(12);
		pTransaction->MaxFee = Amount(765);

		// Act:
		PublishOne<TransactionFeeNotification>(*pTransaction, [&transaction = *pTransaction](const auto& notification) {
			// Assert: max fee is used when there is no associated block
			EXPECT_EQ(GetSignerAddress(transaction), notification.Sender);
			EXPECT_EQ(transaction.Size, notification.TransactionSize);
			EXPECT_EQ(Amount(765), notification.Fee);
			EXPECT_EQ(Amount(765), notification.MaxFee);
		});
	}

	TEST(TEST_CLASS, CanRaiseTransactionFeeNotification_BlockDependent) {
		// Arrange:
		auto hash = test::GenerateRandomByteArray<Hash256>();
		auto pTransaction = test::GenerateRandomTransactionWithSize(234);
		pTransaction->Type = mocks::MockTransaction::Entity_Type;
		pTransaction->MaxFee = Amount(765);

		BlockHeader blockHeader;
		blockHeader.FeeMultiplier = BlockFeeMultiplier(4);
		auto weakEntityInfo = WeakEntityInfo(*pTransaction, hash, blockHeader);

		// Act:
		PublishOne<TransactionFeeNotification>(weakEntityInfo, [&transaction = *pTransaction](const auto& notification) {
			// Assert: calculated fee is used when there is associated block
			EXPECT_EQ(GetSignerAddress(transaction), notification.Sender);
			EXPECT_EQ(transaction.Size, notification.TransactionSize);
			EXPECT_EQ(Amount(4 * 234), notification.Fee);
			EXPECT_EQ(Amount(765), notification.MaxFee);
		});
	}

	TEST(TEST_CLASS, CanRaiseTransactionFeeDebitNotifications) {
		// Arrange:
		auto pTransaction = mocks::CreateMockTransaction(12);
		test::FillWithRandomData(pTransaction->SignerPublicKey);
		pTransaction->MaxFee = Amount(765);

		// Act:
		PublishOne<BalanceDebitNotification>(*pTransaction, [&transaction = *pTransaction](const auto& notification) {
			// Assert:
			EXPECT_EQ(GetSignerAddress(transaction), notification.Sender);
			EXPECT_EQ(Currency_Mosaic_Id, notification.MosaicId);
			EXPECT_EQ(Amount(765), notification.Amount);
		});
	}

	namespace {
		void AssertCustomTransactionNotifications(const std::vector<NotificationType>& notificationTypes, size_t startIndex) {
			EXPECT_EQ(Core_Register_Account_Public_Key_Notification, notificationTypes[startIndex]);
			EXPECT_EQ(mocks::Mock_Address_Notification, notificationTypes[startIndex + 1]);
			EXPECT_EQ(mocks::Mock_Observer_1_Notification, notificationTypes[startIndex + 2]);
			EXPECT_EQ(mocks::Mock_Validator_1_Notification, notificationTypes[startIndex + 3]);
			EXPECT_EQ(mocks::Mock_All_1_Notification, notificationTypes[startIndex + 4]);
			EXPECT_EQ(mocks::Mock_Observer_2_Notification, notificationTypes[startIndex + 5]);
			EXPECT_EQ(mocks::Mock_Validator_2_Notification, notificationTypes[startIndex + 6]);
			EXPECT_EQ(mocks::Mock_All_2_Notification, notificationTypes[startIndex + 7]);
			EXPECT_EQ(mocks::Mock_Hash_Notification, notificationTypes[startIndex + 8]);
		}
	}

	TEST(TEST_CLASS, CanRaiseCustomTransactionNotifications) {
		// Arrange:
		auto pTransaction = mocks::CreateMockTransaction(12);

		// Act:
		PublishAll(*pTransaction, [&transaction = *pTransaction](const auto& sub) {
			// Assert: 8 raised by NotificationPublisher, 9 raised by MockTransaction::publish
			ASSERT_EQ(8u + 9, sub.numNotifications());
			AssertCustomTransactionNotifications(sub.notificationTypes(), 8);
		});
	}

	TEST(TEST_CLASS, CanRaiseCustomTransactionNotificationsDependentOnHash) {
		// Arrange:
		auto hash = test::GenerateRandomByteArray<Hash256>();
		auto pTransaction = mocks::CreateMockTransaction(12);

		// Act:
		PublishOne<mocks::MockHashNotification>(*pTransaction, hash, [&hash](const auto& notification) {
			// Assert:
			EXPECT_EQ(&hash, &notification.Hash);
		});
	}

	TEST(TEST_CLASS, CanRaiseCustomTransactionNotificationsDependentOnSignerAddress) {
		// Arrange:
		auto pTransaction = mocks::CreateMockTransaction(12);
		auto signerAddress = GetSignerAddress(*pTransaction);

		// Act:
		PublishOne<mocks::MockAddressNotification>(*pTransaction, [&signerAddress](const auto& notification) {
			// Assert:
			EXPECT_EQ(signerAddress, notification.Address);
		});
	}

	TEST(TEST_CLASS, CanPublishTransactionNotificationsWithModeBasic) {
		// Arrange:
		auto pTransaction = mocks::CreateMockTransaction(12);

		// Act:
		PublishAll(*pTransaction, PublicationMode::Basic, [&transaction = *pTransaction](const auto& sub) {
			// Assert: 8 raised by NotificationPublisher, none raised by MockTransaction::publish
			ASSERT_EQ(8u, sub.numNotifications());
			EXPECT_EQ(Core_Source_Change_Notification, sub.notificationTypes()[0]);
			EXPECT_EQ(Core_Register_Account_Public_Key_Notification, sub.notificationTypes()[1]);
			EXPECT_EQ(Core_Entity_Notification, sub.notificationTypes()[2]);
			EXPECT_EQ(Core_Transaction_Notification, sub.notificationTypes()[3]);
			EXPECT_EQ(Core_Transaction_Deadline_Notification, sub.notificationTypes()[4]);
			EXPECT_EQ(Core_Transaction_Fee_Notification, sub.notificationTypes()[5]);
			EXPECT_EQ(Core_Balance_Debit_Notification, sub.notificationTypes()[6]);
			EXPECT_EQ(Core_Signature_Notification, sub.notificationTypes()[7]);
		});
	}

	TEST(TEST_CLASS, CanPublishTransactionNotificationsWithModeCustom) {
		// Arrange:
		auto pTransaction = mocks::CreateMockTransaction(12);

		// Act:
		PublishAll(*pTransaction, PublicationMode::Custom, [&transaction = *pTransaction](const auto& sub) {
			// Assert: 9 raised by MockTransaction::publish
			ASSERT_EQ(9u, sub.numNotifications());
			AssertCustomTransactionNotifications(sub.notificationTypes(), 0);
		});
	}

	// endregion

	// region other

	TEST(TEST_CLASS, CannotRaiseAnyNotificationsForUnknownEntities) {
		// Arrange:
		VerifiableEntity entity{};

		// Act:
		EXPECT_THROW(PublishOne<SourceChangeNotification>(entity, [](const auto&) {}), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CannotRaiseAnyNotificationsForUnknownEntitiesWithModeBasic) {
		// Arrange:
		VerifiableEntity entity{};

		// Act:
		EXPECT_THROW(PublishAll(entity, PublicationMode::Basic, [](const auto&) {}), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CannotRaiseAnyNotificationsForUnknownEntitiesWithModeCustom) {
		// Arrange:
		VerifiableEntity entity{};

		// Act:
		EXPECT_THROW(PublishAll(entity, PublicationMode::Custom, [](const auto&) {}), catapult_runtime_error);
	}

	// endregion
}}
