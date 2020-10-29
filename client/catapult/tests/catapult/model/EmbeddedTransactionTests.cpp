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

#include "catapult/model/EmbeddedTransaction.h"
#include "catapult/model/Address.h"
#include "catapult/preprocessor.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS EmbeddedTransactionTests

	// region size + alignment

#define EMBEDDED_TRANSACTION_FIELDS FIELD(SignerPublicKey) FIELD(Version) FIELD(Network) FIELD(Type)

	TEST(TEST_CLASS, TransactionHasExpectedSize) {
		// Arrange:
		auto expectedSize = sizeof(SizePrefixedEntity) + 2 * sizeof(uint32_t);

#define FIELD(X) expectedSize += SizeOf32<decltype(EmbeddedTransaction::X)>();
		EMBEDDED_TRANSACTION_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(EmbeddedTransaction));
		EXPECT_EQ(4u + 8 + 36, sizeof(EmbeddedTransaction));
	}

	TEST(TEST_CLASS, TransactionHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(EmbeddedTransaction, X);
		EMBEDDED_TRANSACTION_FIELDS
#undef FIELD

		EXPECT_EQ(0u, sizeof(EmbeddedTransaction) % 8);
	}

#undef EMBEDDED_TRANSACTION_FIELDS

	// endregion

	// region insertion operator

	TEST(TEST_CLASS, CanOutputTransaction) {
		// Arrange:
		EmbeddedTransaction transaction;
		transaction.Size = 121;
		transaction.Version = 2;
		transaction.Type = static_cast<EntityType>(0x1234);

		// Act:
		auto str = test::ToString(transaction);

		// Assert:
		EXPECT_EQ("(embedded) EntityType<0x1234> (v2) with size 121", str);
	}

	// endregion

	// region GetSignerAddress

	TEST(TEST_CLASS, GetSignerAddressCalculatesCorrectSignerAddress) {
		// Arrange:
		EmbeddedTransaction transaction;
		test::FillWithRandomData(transaction.SignerPublicKey);
		transaction.Network = static_cast<NetworkIdentifier>(test::RandomByte());

		// Act:
		auto signerAddress = GetSignerAddress(transaction);

		// Assert:
		auto expectedSignerAddress = PublicKeyToAddress(transaction.SignerPublicKey, transaction.Network);
		EXPECT_EQ(expectedSignerAddress, signerAddress);
	}

	// endregion

	// region IsSizeValid

	namespace {
		bool IsSizeValid(const EmbeddedTransaction& transaction, bool isEmbeddable = true) {
			auto options = isEmbeddable ? mocks::PluginOptionFlags::Default : mocks::PluginOptionFlags::Not_Embeddable;
			auto registry = mocks::CreateDefaultTransactionRegistry(options);
			return IsSizeValid(transaction, registry);
		}
	}

	TEST(TEST_CLASS, SizeIsInvalidForTransactionWithUnknownType) {
		// Arrange:
		EmbeddedTransaction transaction;
		transaction.Type = static_cast<EntityType>(std::numeric_limits<uint16_t>::max());
		transaction.Size = sizeof(EmbeddedTransaction);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(transaction));
	}

	namespace {
		std::unique_ptr<EmbeddedTransaction> CreateMockEmbeddedTransaction(uint32_t delta) {
			auto pTransaction = mocks::CreateEmbeddedMockTransaction(7);
			pTransaction->Size += delta;
			return PORTABLE_MOVE(pTransaction);
		}
	}

	TEST(TEST_CLASS, SizeIsInvalidForTransactionThatDoesNotSupportEmbedding) {
		// Arrange:
		auto pTransaction = CreateMockEmbeddedTransaction(0);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pTransaction, false));
	}

	TEST(TEST_CLASS, SizeIsValidForTransactionWithEqualReportedSizeAndActualSize) {
		// Arrange:
		auto pTransaction = CreateMockEmbeddedTransaction(0);

		// Act + Assert:
		EXPECT_TRUE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeIsInvalidForTransactionWithReportedSizeLessThanActualSize) {
		// Arrange:
		auto pTransaction = CreateMockEmbeddedTransaction(static_cast<uint32_t>(-1));

		// Act:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeIsInvalidForTransactionWithReportedSizeGreaterThanActualSize) {
		// Arrange:
		auto pTransaction = CreateMockEmbeddedTransaction(1);

		// Act + Assert:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeIsInvalidForTransactionWithReportedSizeLessThanHeaderSize) {
		// Arrange:
		std::vector<uint8_t> buffer(sizeof(SizePrefixedEntity));
		auto* pTransaction = reinterpret_cast<EmbeddedTransaction*>(&buffer[0]);
		pTransaction->Size = sizeof(SizePrefixedEntity);

		// Act:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	TEST(TEST_CLASS, SizeIsInvalidForTransactionWithReportedSizeLessThanDerivedHeaderSize) {
		// Arrange:
		auto pTransaction = std::make_unique<EmbeddedTransaction>();
		pTransaction->Type = mocks::MockTransaction::Entity_Type;
		pTransaction->Size = sizeof(EmbeddedTransaction);

		// Act:
		EXPECT_FALSE(IsSizeValid(*pTransaction));
	}

	// endregion

	// region PublishNotifications

	TEST(TEST_CLASS, PublishNotificationsPublishesAccountNotifications) {
		// Arrange:
		EmbeddedTransaction transaction;
		transaction.Size = sizeof(EmbeddedTransaction);
		test::FillWithRandomData(transaction.SignerPublicKey);
		mocks::MockNotificationSubscriber sub;

		// Act:
		PublishNotifications(transaction, sub);

		// Assert:
		EXPECT_EQ(1u, sub.numNotifications());
		EXPECT_EQ(0u, sub.numAddresses());
		EXPECT_EQ(1u, sub.numKeys());

		EXPECT_TRUE(sub.contains(transaction.SignerPublicKey));
	}

	// endregion

	// region AdvanceNext

	TEST(TEST_CLASS, AdvanceNext_CanMoveToNextTransactionWhenCurrentTransactionDoesNotHavePadding) {
		// Arrange:
		std::vector<uint8_t> buffer(120);
		auto* pCurrent = reinterpret_cast<EmbeddedTransaction*>(&buffer[0]);
		pCurrent->Size = 64;

		// Act:
		const auto* pNext = AdvanceNext(pCurrent);

		// Assert:
		EXPECT_EQ(reinterpret_cast<const EmbeddedTransaction*>(&buffer[64]), pNext);
	}

	TEST(TEST_CLASS, AdvanceNext_CanMoveToNextTransactionWhenCurrentTransactionHasPadding) {
		// Arrange:
		std::vector<uint8_t> buffer(120);
		auto* pCurrent = reinterpret_cast<EmbeddedTransaction*>(&buffer[0]);
		pCurrent->Size = 60;

		// Act:
		const auto* pNext = AdvanceNext(pCurrent);

		// Assert:
		EXPECT_EQ(reinterpret_cast<const EmbeddedTransaction*>(&buffer[60 + 4]), pNext);
	}

	// endregion
}}
