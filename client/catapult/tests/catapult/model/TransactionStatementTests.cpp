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

		std::vector<uint8_t> CreateExpectedSerializedHeaderDataForHashTests() {
			return {
				0x01, 0x00, // version
				0x43, 0xE1, // type
				0x22, 0x02, 0x00, 0x00, // source
				0x33, 0x03, 0x00, 0x00
			};
		}
	}

	// endregion

	// region zero attached receipts

	TEST(TEST_CLASS, CanCreateWithZeroAttachedReceipts) {
		// Act:
		auto transactionStatement = TransactionStatement({ 0x222, 0x333 });

		// Assert: source
		EXPECT_EQ(0x222u, transactionStatement.source() .PrimaryId);
		EXPECT_EQ(0x333u, transactionStatement.source() .SecondaryId);

		// - receipts
		EXPECT_EQ(0u, transactionStatement.size());
	}

	TEST(TEST_CLASS, CanCalculateHashWithZeroAttachedReceipts) {
		// Act:
		auto transactionStatement = TransactionStatement({ 0x222, 0x333 });
		auto hash = transactionStatement.hash();

		// Assert:
		Hash256 expectedHash;
		auto expectedSerializedData = CreateExpectedSerializedHeaderDataForHashTests();
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
		EXPECT_EQ(0x222u, transactionStatement.source() .PrimaryId);
		EXPECT_EQ(0x333u, transactionStatement.source() .SecondaryId);

		// - receipts
		ASSERT_EQ(1u, transactionStatement.size());
		EXPECT_EQ_MEMORY(&receipt, &transactionStatement.receiptAt(0), receipt.Size);
	}

	TEST(TEST_CLASS, CanCalculateHashWithSingleAttachedReceipt) {
		// Act:
		auto transactionStatement = TransactionStatement({ 0x222, 0x333 });
		transactionStatement.addReceipt(CustomReceipt<4>(std::array<uint8_t, 4>{ { 0xAB, 0xFA, 0xCE, 0x55 } }));
		auto hash = transactionStatement.hash();

		// Assert:
		Hash256 expectedHash;
		auto expectedSerializedData = CreateExpectedSerializedHeaderDataForHashTests();
		std::vector<uint8_t> receiptSerializedData{
			0x02, 0x00,
			0x04, 0x00,
			0xAB, 0xFA, 0xCE, 0x55
		};
		expectedSerializedData.insert(expectedSerializedData.end(), receiptSerializedData.cbegin(), receiptSerializedData.cend());
		crypto::Sha3_256(expectedSerializedData, expectedHash);

		EXPECT_EQ(expectedHash, hash);
	}

	// endregion

	// region multiple attached (unique) receipts

	TEST(TEST_CLASS, CanCreateWithMultipleAttachedReceipts) {
		// Arrange:
		CustomReceipt<4> receipt1(std::array<uint8_t, 4>{ { 0xAB, 0xFA, 0xCE, 0x55 } });
		CustomReceipt<5> receipt2(std::array<uint8_t, 5>{ { 0x23, 0x03, 0x82, 0x94, 0x70 } });
		CustomReceipt<3> receipt3(std::array<uint8_t, 3>{ { 0x39, 0x62, 0x19 } });

		// Act:
		auto transactionStatement = TransactionStatement({ 0x222, 0x333 });
		transactionStatement.addReceipt(receipt1);
		transactionStatement.addReceipt(receipt2);
		transactionStatement.addReceipt(receipt3);

		// Assert: source
		EXPECT_EQ(0x222u, transactionStatement.source() .PrimaryId);
		EXPECT_EQ(0x333u, transactionStatement.source() .SecondaryId);

		// - receipts
		ASSERT_EQ(3u, transactionStatement.size());
		EXPECT_EQ_MEMORY(&receipt1, &transactionStatement.receiptAt(0), receipt1.Size);
		EXPECT_EQ_MEMORY(&receipt2, &transactionStatement.receiptAt(1), receipt2.Size);
		EXPECT_EQ_MEMORY(&receipt3, &transactionStatement.receiptAt(2), receipt3.Size);
	}

	TEST(TEST_CLASS, CanCalculateHashWithMultipleAttachedReceipts) {
		// Act:
		auto transactionStatement = TransactionStatement({ 0x222, 0x333 });
		transactionStatement.addReceipt(CustomReceipt<4>(std::array<uint8_t, 4>{ { 0xAB, 0xFA, 0xCE, 0x55 } }));
		transactionStatement.addReceipt(CustomReceipt<5>(std::array<uint8_t, 5>{ { 0x23, 0x03, 0x82, 0x94, 0x70 } }));
		transactionStatement.addReceipt(CustomReceipt<3>(std::array<uint8_t, 3>{ { 0x39, 0x62, 0x19 } }));
		auto hash = transactionStatement.hash();

		// Assert:
		Hash256 expectedHash;
		auto expectedSerializedData = CreateExpectedSerializedHeaderDataForHashTests();
		std::vector<uint8_t> receipt1SerializedData{
			0x02, 0x00,
			0x04, 0x00,
			0xAB, 0xFA, 0xCE, 0x55
		};
		std::vector<uint8_t> receipt2SerializedData{
			0x02, 0x00,
			0x05, 0x00,
			0x23, 0x03, 0x82, 0x94, 0x70,
		};
		std::vector<uint8_t> receipt3SerializedData{
			0x02, 0x00,
			0x03, 0x00,
			0x39, 0x62, 0x19
		};
		expectedSerializedData.insert(expectedSerializedData.end(), receipt1SerializedData.cbegin(), receipt1SerializedData.cend());
		expectedSerializedData.insert(expectedSerializedData.end(), receipt2SerializedData.cbegin(), receipt2SerializedData.cend());
		expectedSerializedData.insert(expectedSerializedData.end(), receipt3SerializedData.cbegin(), receipt3SerializedData.cend());
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
		EXPECT_EQ(0x222u, transactionStatement.source() .PrimaryId);
		EXPECT_EQ(0x333u, transactionStatement.source() .SecondaryId);

		// - receipts
		ASSERT_EQ(3u, transactionStatement.size());
		EXPECT_EQ_MEMORY(&receipt, &transactionStatement.receiptAt(0), receipt.Size);
		EXPECT_EQ_MEMORY(&receipt, &transactionStatement.receiptAt(1), receipt.Size);
		EXPECT_EQ_MEMORY(&receipt, &transactionStatement.receiptAt(2), receipt.Size);
	}

	TEST(TEST_CLASS, CanCalculateHashWithMultipleDuplicateAttachedReceipts) {
		// Act:
		auto transactionStatement = TransactionStatement({ 0x222, 0x333 });
		transactionStatement.addReceipt(CustomReceipt<4>(std::array<uint8_t, 4>{ { 0xAB, 0xFA, 0xCE, 0x55 } }));
		transactionStatement.addReceipt(CustomReceipt<4>(std::array<uint8_t, 4>{ { 0xAB, 0xFA, 0xCE, 0x55 } }));
		transactionStatement.addReceipt(CustomReceipt<4>(std::array<uint8_t, 4>{ { 0xAB, 0xFA, 0xCE, 0x55 } }));
		auto hash = transactionStatement.hash();

		// Assert:
		Hash256 expectedHash;
		auto expectedSerializedData = CreateExpectedSerializedHeaderDataForHashTests();
		std::vector<uint8_t> receiptSerializedData{
			0x02, 0x00,
			0x04, 0x00,
			0xAB, 0xFA, 0xCE, 0x55
		};
		expectedSerializedData.insert(expectedSerializedData.end(), receiptSerializedData.cbegin(), receiptSerializedData.cend());
		expectedSerializedData.insert(expectedSerializedData.end(), receiptSerializedData.cbegin(), receiptSerializedData.cend());
		expectedSerializedData.insert(expectedSerializedData.end(), receiptSerializedData.cbegin(), receiptSerializedData.cend());
		crypto::Sha3_256(expectedSerializedData, expectedHash);

		EXPECT_EQ(expectedHash, hash);
	}

	// endregion
}}
