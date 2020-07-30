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

#include "catapult/model/Receipt.h"
#include "tests/test/nodeps/Alignment.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS ReceiptTests

	// region Receipt

#define RECEIPT_FIELDS FIELD(Version) FIELD(Type)

	TEST(TEST_CLASS, ReceiptHasExpectedSize) {
		// Arrange:
		auto expectedSize = sizeof(SizePrefixedEntity);

#define FIELD(X) expectedSize += SizeOf32<decltype(Receipt::X)>();
		RECEIPT_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(Receipt));
		EXPECT_EQ(4u + 4, sizeof(Receipt));
	}

	TEST(TEST_CLASS, ReceiptHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(Receipt, X);
		RECEIPT_FIELDS
#undef FIELD
	}

#undef RECEIPT_FIELDS

	// endregion

	// region BalanceTransferReceipt

#define RECEIPT_FIELDS FIELD(Mosaic) FIELD(SenderAddress) FIELD(RecipientAddress)

	TEST(TEST_CLASS, BalanceTransferReceiptHasExpectedSize) {
		// Arrange:
		auto expectedSize = sizeof(Receipt);

#define FIELD(X) expectedSize += SizeOf32<decltype(BalanceTransferReceipt::X)>();
		RECEIPT_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(BalanceTransferReceipt));
		EXPECT_EQ(8u + 64, sizeof(BalanceTransferReceipt));
	}

	TEST(TEST_CLASS, BalanceTransferReceiptHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(BalanceTransferReceipt, X);
		RECEIPT_FIELDS
#undef FIELD
	}

#undef RECEIPT_FIELDS

	TEST(TEST_CLASS, CanCreateBalanceTransferReceipt) {
		// Arrange:
		auto sender = test::GenerateRandomByteArray<Address>();
		auto recipient = test::GenerateRandomByteArray<Address>();

		// Act:
		BalanceTransferReceipt receipt(static_cast<ReceiptType>(123), sender, recipient, MosaicId(88), Amount(452));

		// Assert:
		ASSERT_EQ(sizeof(BalanceTransferReceipt), receipt.Size);
		EXPECT_EQ(1u, receipt.Version);
		EXPECT_EQ(static_cast<ReceiptType>(123), receipt.Type);
		EXPECT_EQ(MosaicId(88), receipt.Mosaic.MosaicId);
		EXPECT_EQ(Amount(452), receipt.Mosaic.Amount);
		EXPECT_EQ(sender, receipt.SenderAddress);
		EXPECT_EQ(recipient, receipt.RecipientAddress);
	}

	// endregion

	// region BalanceChangeReceipt

#define RECEIPT_FIELDS FIELD(Mosaic) FIELD(TargetAddress)

	TEST(TEST_CLASS, BalanceChangeReceiptHasExpectedSize) {
		// Arrange:
		auto expectedSize = sizeof(Receipt);

#define FIELD(X) expectedSize += SizeOf32<decltype(BalanceChangeReceipt::X)>();
		RECEIPT_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(BalanceChangeReceipt));
		EXPECT_EQ(8u + 40, sizeof(BalanceChangeReceipt));
	}

	TEST(TEST_CLASS, BalanceChangeReceiptHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(BalanceChangeReceipt, X);
		RECEIPT_FIELDS
#undef FIELD
	}

#undef RECEIPT_FIELDS

	TEST(TEST_CLASS, CanCreateBalanceChangeReceipt) {
		// Arrange:
		auto target = test::GenerateRandomByteArray<Address>();

		// Act:
		BalanceChangeReceipt receipt(static_cast<ReceiptType>(124), target, MosaicId(88), Amount(452));

		// Assert:
		ASSERT_EQ(sizeof(BalanceChangeReceipt), receipt.Size);
		EXPECT_EQ(1u, receipt.Version);
		EXPECT_EQ(static_cast<ReceiptType>(124), receipt.Type);
		EXPECT_EQ(MosaicId(88), receipt.Mosaic.MosaicId);
		EXPECT_EQ(Amount(452), receipt.Mosaic.Amount);
		EXPECT_EQ(target, receipt.TargetAddress);
	}

	// endregion

	// region InflationReceipt

#define RECEIPT_FIELDS FIELD(Mosaic)

	TEST(TEST_CLASS, InflationReceiptHasExpectedSize) {
		// Arrange:
		auto expectedSize = sizeof(Receipt);

#define FIELD(X) expectedSize += SizeOf32<decltype(InflationReceipt::X)>();
		RECEIPT_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(InflationReceipt));
		EXPECT_EQ(8u + 16, sizeof(InflationReceipt));
	}

	TEST(TEST_CLASS, InflationReceiptHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(InflationReceipt, X);
		RECEIPT_FIELDS
#undef FIELD
	}

#undef RECEIPT_FIELDS

	TEST(TEST_CLASS, CanCreateInflationReceipt) {
		// Act:
		InflationReceipt receipt(static_cast<ReceiptType>(124), MosaicId(88), Amount(452));

		// Assert:
		ASSERT_EQ(sizeof(InflationReceipt), receipt.Size);
		EXPECT_EQ(1u, receipt.Version);
		EXPECT_EQ(static_cast<ReceiptType>(124), receipt.Type);
		EXPECT_EQ(MosaicId(88), receipt.Mosaic.MosaicId);
		EXPECT_EQ(Amount(452), receipt.Mosaic.Amount);
	}

	// endregion

	// region ArtifactExpiryReceipt

#define RECEIPT_FIELDS FIELD(ArtifactId)

	TEST(TEST_CLASS, ArtifactExpiryReceiptHasExpectedSize) {
		// Arrange:
		auto expectedSize = sizeof(Receipt);

#define FIELD(X) expectedSize += SizeOf32<decltype(ArtifactExpiryReceipt<uint64_t>::X)>();
		RECEIPT_FIELDS
#undef FIELD

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(ArtifactExpiryReceipt<uint64_t>));
		EXPECT_EQ(8u + 8, sizeof(ArtifactExpiryReceipt<uint64_t>));
	}

	TEST(TEST_CLASS, ArtifactExpiryReceiptHasProperAlignment) {
#define FIELD(X) EXPECT_ALIGNED(ArtifactExpiryReceipt<uint64_t>, X);
		RECEIPT_FIELDS
#undef FIELD
	}

#undef RECEIPT_FIELDS

	TEST(TEST_CLASS, CanCreateArtifactExpiryReceipt) {
		// Act:
		ArtifactExpiryReceipt<uint64_t> receipt(static_cast<ReceiptType>(125), 8899);

		// Assert:
		ASSERT_EQ(sizeof(ArtifactExpiryReceipt<uint64_t>), receipt.Size);
		EXPECT_EQ(1u, receipt.Version);
		EXPECT_EQ(static_cast<ReceiptType>(125), receipt.Type);
		EXPECT_EQ(8899u, receipt.ArtifactId);
	}

	// endregion
}}
