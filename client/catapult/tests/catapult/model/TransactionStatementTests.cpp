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

#include "catapult/model/TransactionStatement.h"
#include "catapult/crypto/Hashes.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS TransactionStatementTests

	// region test utils

	namespace {
#pragma pack(push, 1)

		template<size_t N>
		struct CustomReceipt : public Receipt {
		public:
			explicit CustomReceipt(const std::array<uint8_t, N>& payload) : Payload(payload) {
				Size = sizeof(Receipt) + N;
				Version = 2;
				Type = static_cast<ReceiptType>(N);
			}

		public:
			std::array<uint8_t, N> Payload;
		};

#pragma pack(pop)

		std::vector<uint8_t> CreateExpectedSerializedDataForHashTests(const std::vector<uint8_t>& receiptsData) {
			std::vector<uint8_t> allData{
				0x01, 0x00, // version
				0x43, 0xE1, // type
				0x22, 0x02, 0x00, 0x00, // source
				0x33, 0x03, 0x00, 0x00
			};

			allData.insert(allData.end(), receiptsData.cbegin(), receiptsData.cend());
			return allData;
		}
	}

	// endregion

	// region zero attached receipts

	TEST(TEST_CLASS, CanCreateWithZeroAttachedReceipts) {
		// Act:
		auto transactionStatement = TransactionStatement({ 0x222, 0x333 });

		// Assert: source
		EXPECT_EQ(0x222u, transactionStatement.source().PrimaryId);
		EXPECT_EQ(0x333u, transactionStatement.source().SecondaryId);

		// - receipts
		EXPECT_EQ(0u, transactionStatement.size());
	}

	TEST(TEST_CLASS, CanCalculateHashWithZeroAttachedReceipts) {
		// Arrange:
		auto transactionStatement = TransactionStatement({ 0x222, 0x333 });

		// Act:
		auto hash = transactionStatement.hash();

		// Assert:
		Hash256 expectedHash;
		auto expectedSerializedData = CreateExpectedSerializedDataForHashTests({});
		crypto::Sha3_256(expectedSerializedData, expectedHash);

		EXPECT_EQ(expectedHash, hash);
	}

	// endregion

	// region single attached receipt

	TEST(TEST_CLASS, CanCreateWithSingleAttachedReceipt) {
		// Arrange:
		CustomReceipt<4> receipt(std::array<uint8_t, 4>{ { 0xAB, 0xFA, 0xCE, 0x55 } });

		// Act:
		auto transactionStatement = TransactionStatement({ 0x222, 0x333 });
		transactionStatement.addReceipt(receipt);

		// Assert: source
		EXPECT_EQ(0x222u, transactionStatement.source().PrimaryId);
		EXPECT_EQ(0x333u, transactionStatement.source().SecondaryId);

		// - receipts
		ASSERT_EQ(1u, transactionStatement.size());
		EXPECT_EQ_MEMORY(&receipt, &transactionStatement.receiptAt(0), receipt.Size);
	}

	TEST(TEST_CLASS, CanCalculateHashWithSingleAttachedReceipt) {
		// Arrange:
		auto transactionStatement = TransactionStatement({ 0x222, 0x333 });
		transactionStatement.addReceipt(CustomReceipt<4>(std::array<uint8_t, 4>{ { 0xAB, 0xFA, 0xCE, 0x55 } }));

		// Act:
		auto hash = transactionStatement.hash();

		// Assert:
		Hash256 expectedHash;
		auto expectedSerializedData = CreateExpectedSerializedDataForHashTests({
			0x02, 0x00, 0x04, 0x00, 0xAB, 0xFA, 0xCE, 0x55
		});
		crypto::Sha3_256(expectedSerializedData, expectedHash);

		EXPECT_EQ(expectedHash, hash);
	}

	// endregion

	// region multiple attached (duplicate) receipts

	TEST(TEST_CLASS, CanCreateWithMultipleDuplicateAttachedReceipts) {
		// Arrange:
		CustomReceipt<4> receipt(std::array<uint8_t, 4>{ { 0xAB, 0xFA, 0xCE, 0x55 } });

		// Act:
		auto transactionStatement = TransactionStatement({ 0x222, 0x333 });
		transactionStatement.addReceipt(receipt);
		transactionStatement.addReceipt(receipt);
		transactionStatement.addReceipt(receipt);

		// Assert: source
		EXPECT_EQ(0x222u, transactionStatement.source().PrimaryId);
		EXPECT_EQ(0x333u, transactionStatement.source().SecondaryId);

		// - receipts
		ASSERT_EQ(3u, transactionStatement.size());
		EXPECT_EQ_MEMORY(&receipt, &transactionStatement.receiptAt(0), receipt.Size);
		EXPECT_EQ_MEMORY(&receipt, &transactionStatement.receiptAt(1), receipt.Size);
		EXPECT_EQ_MEMORY(&receipt, &transactionStatement.receiptAt(2), receipt.Size);
	}

	TEST(TEST_CLASS, CanCalculateHashWithMultipleDuplicateAttachedReceipts) {
		// Arrange:
		auto transactionStatement = TransactionStatement({ 0x222, 0x333 });
		transactionStatement.addReceipt(CustomReceipt<4>(std::array<uint8_t, 4>{ { 0xAB, 0xFA, 0xCE, 0x55 } }));
		transactionStatement.addReceipt(CustomReceipt<4>(std::array<uint8_t, 4>{ { 0xAB, 0xFA, 0xCE, 0x55 } }));
		transactionStatement.addReceipt(CustomReceipt<4>(std::array<uint8_t, 4>{ { 0xAB, 0xFA, 0xCE, 0x55 } }));

		// Act:
		auto hash = transactionStatement.hash();

		// Assert:
		Hash256 expectedHash;
		auto expectedSerializedData = CreateExpectedSerializedDataForHashTests({
			0x02, 0x00, 0x04, 0x00, 0xAB, 0xFA, 0xCE, 0x55,
			0x02, 0x00, 0x04, 0x00, 0xAB, 0xFA, 0xCE, 0x55,
			0x02, 0x00, 0x04, 0x00, 0xAB, 0xFA, 0xCE, 0x55
		});
		crypto::Sha3_256(expectedSerializedData, expectedHash);

		EXPECT_EQ(expectedHash, hash);
	}

	// endregion

	// region multiple attached (unique + in order) receipts

	TEST(TEST_CLASS, CanCreateWithMultipleAttachedReceipts_InOrder) {
		// Arrange:
		CustomReceipt<3> receipt1(std::array<uint8_t, 3>{ { 0x39, 0x62, 0x19 } });
		CustomReceipt<4> receipt2(std::array<uint8_t, 4>{ { 0xAB, 0xFA, 0xCE, 0x55 } });
		CustomReceipt<5> receipt3(std::array<uint8_t, 5>{ { 0x23, 0x03, 0x82, 0x94, 0x70 } });

		// Act:
		auto transactionStatement = TransactionStatement({ 0x222, 0x333 });
		transactionStatement.addReceipt(receipt1);
		transactionStatement.addReceipt(receipt2);
		transactionStatement.addReceipt(receipt3);

		// Assert: source
		EXPECT_EQ(0x222u, transactionStatement.source().PrimaryId);
		EXPECT_EQ(0x333u, transactionStatement.source().SecondaryId);

		// - receipts
		ASSERT_EQ(3u, transactionStatement.size());
		EXPECT_EQ_MEMORY(&receipt1, &transactionStatement.receiptAt(0), receipt1.Size);
		EXPECT_EQ_MEMORY(&receipt2, &transactionStatement.receiptAt(1), receipt2.Size);
		EXPECT_EQ_MEMORY(&receipt3, &transactionStatement.receiptAt(2), receipt3.Size);
	}

	TEST(TEST_CLASS, CanCalculateHashWithMultipleAttachedReceipts_InOrder) {
		// Arrange:
		auto transactionStatement = TransactionStatement({ 0x222, 0x333 });
		transactionStatement.addReceipt(CustomReceipt<3>(std::array<uint8_t, 3>{ { 0x39, 0x62, 0x19 } }));
		transactionStatement.addReceipt(CustomReceipt<4>(std::array<uint8_t, 4>{ { 0xAB, 0xFA, 0xCE, 0x55 } }));
		transactionStatement.addReceipt(CustomReceipt<5>(std::array<uint8_t, 5>{ { 0x23, 0x03, 0x82, 0x94, 0x70 } }));

		// Act:
		auto hash = transactionStatement.hash();

		// Assert:
		Hash256 expectedHash;
		auto expectedSerializedData = CreateExpectedSerializedDataForHashTests({
			0x02, 0x00, 0x03, 0x00, 0x39, 0x62, 0x19,
			0x02, 0x00, 0x04, 0x00, 0xAB, 0xFA, 0xCE, 0x55,
			0x02, 0x00, 0x05, 0x00, 0x23, 0x03, 0x82, 0x94, 0x70
		});
		crypto::Sha3_256(expectedSerializedData, expectedHash);

		EXPECT_EQ(expectedHash, hash);
	}

	// endregion

	// region multiple attached (unique + out of order) receipts

	TEST(TEST_CLASS, CanCreateWithMultipleAttachedReceipts_OutOfOrder) {
		// Arrange:
		CustomReceipt<3> receipt1(std::array<uint8_t, 3>{ { 0x39, 0x62, 0x19 } });
		CustomReceipt<5> receipt2(std::array<uint8_t, 5>{ { 0x23, 0x03, 0x82, 0x94, 0x70 } });
		CustomReceipt<4> receipt3(std::array<uint8_t, 4>{ { 0xAB, 0xFA, 0xCE, 0x55 } });
		CustomReceipt<3> receipt4(std::array<uint8_t, 3>{ { 0x00, 0xDD, 0xDD } });
		CustomReceipt<4> receipt5(std::array<uint8_t, 4>{ { 0x00, 0xCC, 0xCC, 0xCC } });
		CustomReceipt<3> receipt6(std::array<uint8_t, 3>{ { 0x00, 0xBB, 0xBB } });

		// Act:
		auto transactionStatement = TransactionStatement({ 0x222, 0x333 });
		transactionStatement.addReceipt(receipt1);
		transactionStatement.addReceipt(receipt2);
		transactionStatement.addReceipt(receipt3);
		transactionStatement.addReceipt(receipt4);
		transactionStatement.addReceipt(receipt5);
		transactionStatement.addReceipt(receipt6);

		// Assert: source
		EXPECT_EQ(0x222u, transactionStatement.source().PrimaryId);
		EXPECT_EQ(0x333u, transactionStatement.source().SecondaryId);

		// - receipts (sorted by type followed by add order)
		ASSERT_EQ(6u, transactionStatement.size());
		EXPECT_EQ_MEMORY(&receipt1, &transactionStatement.receiptAt(0), receipt1.Size);
		EXPECT_EQ_MEMORY(&receipt4, &transactionStatement.receiptAt(1), receipt4.Size);
		EXPECT_EQ_MEMORY(&receipt6, &transactionStatement.receiptAt(2), receipt6.Size);
		EXPECT_EQ_MEMORY(&receipt3, &transactionStatement.receiptAt(3), receipt3.Size);
		EXPECT_EQ_MEMORY(&receipt5, &transactionStatement.receiptAt(4), receipt5.Size);
		EXPECT_EQ_MEMORY(&receipt2, &transactionStatement.receiptAt(5), receipt2.Size);
	}

	TEST(TEST_CLASS, CanCalculateHashWithMultipleAttachedReceipts_OutOfOrder) {
		// Arrange:
		auto transactionStatement = TransactionStatement({ 0x222, 0x333 });
		transactionStatement.addReceipt(CustomReceipt<3>(std::array<uint8_t, 3>{ { 0x39, 0x62, 0x19 } }));
		transactionStatement.addReceipt(CustomReceipt<5>(std::array<uint8_t, 5>{ { 0x23, 0x03, 0x82, 0x94, 0x70 } }));
		transactionStatement.addReceipt(CustomReceipt<4>(std::array<uint8_t, 4>{ { 0xAB, 0xFA, 0xCE, 0x55 } }));
		transactionStatement.addReceipt(CustomReceipt<3>(std::array<uint8_t, 3>{ { 0x00, 0xDD, 0xDD } }));
		transactionStatement.addReceipt(CustomReceipt<4>(std::array<uint8_t, 4>{ { 0x00, 0xCC, 0xCC, 0xCC } }));
		transactionStatement.addReceipt(CustomReceipt<3>(std::array<uint8_t, 3>{ { 0x00, 0xBB, 0xBB } }));

		// Act:
		auto hash = transactionStatement.hash();

		// Assert: receipts are sorted (by type followed by add order) prior to hashing
		Hash256 expectedHash;
		auto expectedSerializedData = CreateExpectedSerializedDataForHashTests({
			0x02, 0x00, 0x03, 0x00, 0x39, 0x62, 0x19,
			0x02, 0x00, 0x03, 0x00, 0x00, 0xDD, 0xDD,
			0x02, 0x00, 0x03, 0x00, 0x00, 0xBB, 0xBB,
			0x02, 0x00, 0x04, 0x00, 0xAB, 0xFA, 0xCE, 0x55,
			0x02, 0x00, 0x04, 0x00, 0x00, 0xCC, 0xCC, 0xCC,
			0x02, 0x00, 0x05, 0x00, 0x23, 0x03, 0x82, 0x94, 0x70
		});
		crypto::Sha3_256(expectedSerializedData, expectedHash);

		EXPECT_EQ(expectedHash, hash);
	}

	// endregion
}}
