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

#include "catapult/model/ResolutionStatement.h"
#include "catapult/crypto/Hashes.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS ResolutionStatementTests

	// region traits

	namespace {
		struct AddressTraits {
			using ResolutionStatementType = AddressResolutionStatement;

			static constexpr uint8_t Statement_Type_Code = 1;

			static UnresolvedAddress CreateRandomUnresolved() {
				return test::GenerateRandomUnresolvedAddress();
			}

			static Address CreateRandomResolved() {
				return test::GenerateRandomAddress();
			}

			template<typename TAddress>
			static RawBuffer ToBuffer(const TAddress& address) {
				return { reinterpret_cast<const uint8_t*>(address.data()), address.size() };
			}
		};

		struct MosaicTraits {
			using ResolutionStatementType = MosaicResolutionStatement;

			static constexpr uint8_t Statement_Type_Code = 2;

			static UnresolvedMosaicId CreateRandomUnresolved() {
				return test::GenerateRandomValue<UnresolvedMosaicId>();
			}

			static MosaicId CreateRandomResolved() {
				return test::GenerateRandomValue<MosaicId>();
			}

			template<typename TMosaicId>
			static RawBuffer ToBuffer(const TMosaicId& mosaicId) {
				return { reinterpret_cast<const uint8_t*>(&mosaicId), sizeof(TMosaicId) };
			}
		};
	}

#define RESOLUTION_STATEMENT_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Address) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<AddressTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Mosaic) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MosaicTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	// endregion

	// region test utils

	namespace {
		void Append(std::vector<uint8_t>& buffer1, const RawBuffer& buffer2) {
			buffer1.insert(buffer1.end(), buffer2.pData, buffer2.pData + buffer2.Size);
		}

		void Append(std::vector<uint8_t>& buffer1, const std::vector<uint8_t>& buffer2) {
			buffer1.insert(buffer1.end(), buffer2.cbegin(), buffer2.cend());
		}

		template<typename TResolutionEntry>
		void AssertEqualEntry(const TResolutionEntry& expected, const TResolutionEntry& actual, size_t id) {
			EXPECT_EQ(expected.ResolvedValue, actual.ResolvedValue) << id;
			EXPECT_EQ(expected.Source.PrimaryId, actual.Source.PrimaryId) << id;
			EXPECT_EQ(expected.Source.SecondaryId, actual.Source.SecondaryId) << id;
		}
	}

	// endregion

	// region zero attached entries

	// notice that this is not a "real" scenario because a real resolution statement should have at least one entry

	RESOLUTION_STATEMENT_TEST(CanCreateWithZeroAttachedResolutionEntries) {
		// Act:
		auto unresolvedValue = TTraits::CreateRandomUnresolved();
		auto resolutionStatement = typename TTraits::ResolutionStatementType(unresolvedValue);

		// Assert: header
		EXPECT_EQ(unresolvedValue, resolutionStatement.unresolved());

		// - entries
		EXPECT_EQ(0u, resolutionStatement.size());
	}

	RESOLUTION_STATEMENT_TEST(CanCalculateHashWithZeroAttachedResolutionEntries) {
		// Act:
		auto unresolvedValue = TTraits::CreateRandomUnresolved();
		auto resolutionStatement = typename TTraits::ResolutionStatementType(unresolvedValue);
		auto hash = resolutionStatement.hash();

		// Assert:
		Hash256 expectedHash;
		std::vector<uint8_t> expectedSerializedData{ 0x01, 0x00, 0x43, 0xF0 | TTraits::Statement_Type_Code };
		Append(expectedSerializedData, TTraits::ToBuffer(unresolvedValue));
		crypto::Sha3_256(expectedSerializedData, expectedHash);

		EXPECT_EQ(expectedHash, hash);
	}

	// endregion

	// region single attached entry

	RESOLUTION_STATEMENT_TEST(CanCreateWithSingleAttachedResolutionEntry) {
		// Arrange:
		auto resolvedValue = TTraits::CreateRandomResolved();

		// Act:
		auto unresolvedValue = TTraits::CreateRandomUnresolved();
		auto resolutionStatement = typename TTraits::ResolutionStatementType(unresolvedValue);
		resolutionStatement.addResolution(resolvedValue, { 11, 12 });

		// Assert: header
		EXPECT_EQ(unresolvedValue, resolutionStatement.unresolved());

		// - entries
		ASSERT_EQ(1u, resolutionStatement.size());
		AssertEqualEntry({ { 11, 12 }, resolvedValue }, resolutionStatement.entryAt(0), 0);
	}

	RESOLUTION_STATEMENT_TEST(CanCalculateHashWithSingleAttachedResolutionEntry) {
		// Arrange:
		auto resolvedValue = TTraits::CreateRandomResolved();

		// Act:
		auto unresolvedValue = TTraits::CreateRandomUnresolved();
		auto resolutionStatement = typename TTraits::ResolutionStatementType(unresolvedValue);
		resolutionStatement.addResolution(resolvedValue, { 11, 12 });
		auto hash = resolutionStatement.hash();

		// Assert:
		Hash256 expectedHash;
		std::vector<uint8_t> expectedSerializedData{ 0x01, 0x00, 0x43, 0xF0 | TTraits::Statement_Type_Code };
		Append(expectedSerializedData, TTraits::ToBuffer(unresolvedValue));
		Append(expectedSerializedData, std::vector<uint8_t>{ 0x0B, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00 });
		Append(expectedSerializedData, TTraits::ToBuffer(resolvedValue));
		crypto::Sha3_256(expectedSerializedData, expectedHash);

		EXPECT_EQ(expectedHash, hash);
	}

	// endregion

	// region multiple attached (unique) entries

	RESOLUTION_STATEMENT_TEST(CanCreateWithMultipleAttachedResolutionEntries) {
		// Arrange:
		auto resolvedValue1 = TTraits::CreateRandomResolved();
		auto resolvedValue2 = TTraits::CreateRandomResolved();
		auto resolvedValue3 = TTraits::CreateRandomResolved();

		// Act:
		auto unresolvedValue = TTraits::CreateRandomUnresolved();
		auto resolutionStatement = typename TTraits::ResolutionStatementType(unresolvedValue);
		resolutionStatement.addResolution(resolvedValue1, { 11, 12 });
		resolutionStatement.addResolution(resolvedValue2, { 16, 00 });
		resolutionStatement.addResolution(resolvedValue3, { 16, 01 });

		// Assert: header
		EXPECT_EQ(unresolvedValue, resolutionStatement.unresolved());

		// - entries
		ASSERT_EQ(3u, resolutionStatement.size());
		AssertEqualEntry({ { 11, 12 }, resolvedValue1 }, resolutionStatement.entryAt(0), 0);
		AssertEqualEntry({ { 16, 00 }, resolvedValue2 }, resolutionStatement.entryAt(1), 1);
		AssertEqualEntry({ { 16, 01 }, resolvedValue3 }, resolutionStatement.entryAt(2), 2);

	}

	RESOLUTION_STATEMENT_TEST(CanCalculateHashWithMultipleAttachedResolutionEntries) {
		// Arrange:
		auto resolvedValue1 = TTraits::CreateRandomResolved();
		auto resolvedValue2 = TTraits::CreateRandomResolved();
		auto resolvedValue3 = TTraits::CreateRandomResolved();

		// Act:
		auto unresolvedValue = TTraits::CreateRandomUnresolved();
		auto resolutionStatement = typename TTraits::ResolutionStatementType(unresolvedValue);
		resolutionStatement.addResolution(resolvedValue1, { 11, 12 });
		resolutionStatement.addResolution(resolvedValue2, { 16, 00 });
		resolutionStatement.addResolution(resolvedValue3, { 16, 01 });
		auto hash = resolutionStatement.hash();

		// Assert:
		Hash256 expectedHash;
		std::vector<uint8_t> expectedSerializedData{ 0x01, 0x00, 0x43, 0xF0 | TTraits::Statement_Type_Code };
		Append(expectedSerializedData, TTraits::ToBuffer(unresolvedValue));
		Append(expectedSerializedData, std::vector<uint8_t>{ 0x0B, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00 });
		Append(expectedSerializedData, TTraits::ToBuffer(resolvedValue1));
		Append(expectedSerializedData, std::vector<uint8_t>{ 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
		Append(expectedSerializedData, TTraits::ToBuffer(resolvedValue2));
		Append(expectedSerializedData, std::vector<uint8_t>{ 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00 });
		Append(expectedSerializedData, TTraits::ToBuffer(resolvedValue3));
		crypto::Sha3_256(expectedSerializedData, expectedHash);

		EXPECT_EQ(expectedHash, hash);
	}

	// endregion

	// region multiple attached (duplicate) entries

	RESOLUTION_STATEMENT_TEST(CanCreateWithMultipleDuplicateAttachedResolutionEntries) {
		// Arrange:
		auto resolvedValue1 = TTraits::CreateRandomResolved();
		auto resolvedValue2 = TTraits::CreateRandomResolved();

		// Act:
		auto unresolvedValue = TTraits::CreateRandomUnresolved();
		auto resolutionStatement = typename TTraits::ResolutionStatementType(unresolvedValue);
		resolutionStatement.addResolution(resolvedValue1, { 11, 12 });
		resolutionStatement.addResolution(resolvedValue1, { 11, 12 }); // collapsed
		resolutionStatement.addResolution(resolvedValue1, { 11, 13 }); // collapsed
		resolutionStatement.addResolution(resolvedValue1, { 12, 12 }); // collapsed
		resolutionStatement.addResolution(resolvedValue1, { 15, 16 }); // collapsed
		resolutionStatement.addResolution(resolvedValue2, { 16, 00 });
		resolutionStatement.addResolution(resolvedValue2, { 18, 00 }); // collapsed
		resolutionStatement.addResolution(resolvedValue1, { 19, 00 });

		// Assert: header
		EXPECT_EQ(unresolvedValue, resolutionStatement.unresolved());

		// - entries
		ASSERT_EQ(3u, resolutionStatement.size());
		AssertEqualEntry({ { 11, 12 }, resolvedValue1 }, resolutionStatement.entryAt(0), 0);
		AssertEqualEntry({ { 16, 00 }, resolvedValue2 }, resolutionStatement.entryAt(1), 1);
		AssertEqualEntry({ { 19, 00 }, resolvedValue1 }, resolutionStatement.entryAt(2), 2);
	}

	RESOLUTION_STATEMENT_TEST(CanCalculateHashWithMultipleDuplicateAttachedResolutionEntries) {
		// Arrange:
		auto resolvedValue1 = TTraits::CreateRandomResolved();
		auto resolvedValue2 = TTraits::CreateRandomResolved();

		// Act:
		auto unresolvedValue = TTraits::CreateRandomUnresolved();
		auto resolutionStatement = typename TTraits::ResolutionStatementType(unresolvedValue);
		resolutionStatement.addResolution(resolvedValue1, { 11, 12 });
		resolutionStatement.addResolution(resolvedValue1, { 11, 12 }); // collapsed
		resolutionStatement.addResolution(resolvedValue1, { 11, 13 }); // collapsed
		resolutionStatement.addResolution(resolvedValue1, { 12, 12 }); // collapsed
		resolutionStatement.addResolution(resolvedValue1, { 15, 16 }); // collapsed
		resolutionStatement.addResolution(resolvedValue2, { 16, 00 });
		resolutionStatement.addResolution(resolvedValue2, { 18, 00 }); // collapsed
		resolutionStatement.addResolution(resolvedValue1, { 19, 00 });
		auto hash = resolutionStatement.hash();

		// Assert:
		Hash256 expectedHash;
		std::vector<uint8_t> expectedSerializedData{ 0x01, 0x00, 0x43, 0xF0 | TTraits::Statement_Type_Code };
		Append(expectedSerializedData, TTraits::ToBuffer(unresolvedValue));
		Append(expectedSerializedData, std::vector<uint8_t>{ 0x0B, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00 });
		Append(expectedSerializedData, TTraits::ToBuffer(resolvedValue1));
		Append(expectedSerializedData, std::vector<uint8_t>{ 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
		Append(expectedSerializedData, TTraits::ToBuffer(resolvedValue2));
		Append(expectedSerializedData, std::vector<uint8_t>{ 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
		Append(expectedSerializedData, TTraits::ToBuffer(resolvedValue1));
		crypto::Sha3_256(expectedSerializedData, expectedHash);

		EXPECT_EQ(expectedHash, hash);
	}

	// endregion

	// region resolution order

	RESOLUTION_STATEMENT_TEST(CannotResolveOutOfOrder) {
		// Arrange:
		auto unresolvedValue = TTraits::CreateRandomUnresolved();
		auto resolutionStatement = typename TTraits::ResolutionStatementType(unresolvedValue);
		resolutionStatement.addResolution(TTraits::CreateRandomResolved(), { 11, 12 });

		// Act + Assert:
		EXPECT_THROW(resolutionStatement.addResolution(TTraits::CreateRandomResolved(), { 11, 11 }), catapult_invalid_argument);
		EXPECT_THROW(resolutionStatement.addResolution(TTraits::CreateRandomResolved(), { 10, 12 }), catapult_invalid_argument);
	}

	// endregion
}}
