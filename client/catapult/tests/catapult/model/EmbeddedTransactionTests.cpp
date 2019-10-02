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
#include "catapult/preprocessor.h"
#include "tests/test/core/mocks/MockNotificationSubscriber.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS EmbeddedTransactionTests

	TEST(TEST_CLASS, TransactionHasExpectedSize) {
		// Arrange:
		auto expectedSize =
				sizeof(uint32_t) // size
				+ sizeof(uint16_t) // version
				+ sizeof(uint16_t) // entity type
				+ sizeof(Key); // signer

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(EmbeddedTransaction));
		EXPECT_EQ(40u, sizeof(EmbeddedTransaction));
	}

	// region insertion operator

	TEST(TEST_CLASS, CanOutputTransaction) {
		// Arrange:
		EmbeddedTransaction transaction;
		transaction.Size = 121;
		transaction.Type = static_cast<EntityType>(0x1234);
		transaction.Version = MakeVersion(NetworkIdentifier::Zero, 2);

		// Act:
		auto str = test::ToString(transaction);

		// Assert:
		EXPECT_EQ("(embedded) EntityType<0x1234> (v2) with size 121", str);
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
		transaction.Type = static_cast<EntityType>(-1);
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
}}
