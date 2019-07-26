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
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS ReceiptTests

	// region Receipt

	TEST(TEST_CLASS, ReceiptHasExpectedSize) {
		// Arrange:
		auto expectedSize =
				sizeof(uint32_t) // size
				+ sizeof(uint16_t) // version
				+ sizeof(uint16_t); // type

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(Receipt));
		EXPECT_EQ(8u, sizeof(Receipt));
	}

	// endregion

	// region BalanceTransferReceipt

	TEST(TEST_CLASS, BalanceTransferReceiptHasExpectedSize) {
		// Arrange:
		auto expectedSize =
				sizeof(Receipt) // base
				+ Key::Size // sender
				+ Address::Size // receipient
				+ sizeof(MosaicId) // mosaic id
				+ sizeof(Amount); // amount

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(BalanceTransferReceipt));
		EXPECT_EQ(8u + 73, sizeof(BalanceTransferReceipt));
	}

	TEST(TEST_CLASS, CanCreateBalanceTransferReceipt) {
		// Arrange:
		auto sender = test::GenerateRandomByteArray<Key>();
		auto recipient = test::GenerateRandomByteArray<Address>();

		// Act:
		BalanceTransferReceipt receipt(static_cast<ReceiptType>(123), sender, recipient, MosaicId(88), Amount(452));

		// Assert:
		ASSERT_EQ(sizeof(BalanceTransferReceipt), receipt.Size);
		EXPECT_EQ(1u, receipt.Version);
		EXPECT_EQ(static_cast<ReceiptType>(123), receipt.Type);
		EXPECT_EQ(sender, receipt.Sender);
		EXPECT_EQ(recipient, receipt.Recipient);
		EXPECT_EQ(MosaicId(88), receipt.MosaicId);
		EXPECT_EQ(Amount(452), receipt.Amount);
	}

	// endregion

	// region BalanceChangeReceipt

	TEST(TEST_CLASS, BalanceChangeReceiptHasExpectedSize) {
		// Arrange:
		auto expectedSize =
				sizeof(Receipt) // base
				+ Key::Size // account
				+ sizeof(MosaicId) // mosaic id
				+ sizeof(Amount); // amount

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(BalanceChangeReceipt));
		EXPECT_EQ(8u + 48, sizeof(BalanceChangeReceipt));
	}

	TEST(TEST_CLASS, CanCreateBalanceChangeReceipt) {
		// Arrange:
		auto account = test::GenerateRandomByteArray<Key>();

		// Act:
		BalanceChangeReceipt receipt(static_cast<ReceiptType>(124), account, MosaicId(88), Amount(452));

		// Assert:
		ASSERT_EQ(sizeof(BalanceChangeReceipt), receipt.Size);
		EXPECT_EQ(1u, receipt.Version);
		EXPECT_EQ(static_cast<ReceiptType>(124), receipt.Type);
		EXPECT_EQ(account, receipt.Account);
		EXPECT_EQ(MosaicId(88), receipt.MosaicId);
		EXPECT_EQ(Amount(452), receipt.Amount);
	}

	// endregion

	// region InflationReceipt

	TEST(TEST_CLASS, InflationReceiptHasExpectedSize) {
		// Arrange:
		auto expectedSize =
				sizeof(Receipt) // base
				+ sizeof(MosaicId) // mosaic id
				+ sizeof(Amount); // amount

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(InflationReceipt));
		EXPECT_EQ(8u + 16, sizeof(InflationReceipt));
	}

	TEST(TEST_CLASS, CanCreateInflationReceipt) {
		// Act:
		InflationReceipt receipt(static_cast<ReceiptType>(124), MosaicId(88), Amount(452));

		// Assert:
		ASSERT_EQ(sizeof(InflationReceipt), receipt.Size);
		EXPECT_EQ(1u, receipt.Version);
		EXPECT_EQ(static_cast<ReceiptType>(124), receipt.Type);
		EXPECT_EQ(MosaicId(88), receipt.MosaicId);
		EXPECT_EQ(Amount(452), receipt.Amount);
	}

	// endregion

	// region ArtifactExpiryReceipt

	TEST(TEST_CLASS, ArtifactExpiryReceiptHasExpectedSize) {
		// Arrange:
		auto expectedSize =
				sizeof(Receipt) // base
				+ sizeof(uint64_t); // id

		// Assert:
		EXPECT_EQ(expectedSize, sizeof(ArtifactExpiryReceipt<uint64_t>));
		EXPECT_EQ(8u + 8, sizeof(ArtifactExpiryReceipt<uint64_t>));
	}

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
