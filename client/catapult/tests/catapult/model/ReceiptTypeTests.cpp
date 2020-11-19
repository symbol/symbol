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

#include "catapult/model/ReceiptType.h"
#include "catapult/utils/HexFormatter.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS ReceiptTypeTests

	namespace {
		auto MakeReceiptType(uint8_t basicReceiptType, uint8_t facility, uint8_t code) {
			return MakeReceiptType(static_cast<BasicReceiptType>(basicReceiptType), static_cast<FacilityCode>(facility), code);
		}
	}

	// region MakeReceiptType

	TEST(TEST_CLASS, CanMakeReceiptType) {
		// Assert: zeros
		EXPECT_EQ(static_cast<ReceiptType>(0x00000000), MakeReceiptType(0, 0, 0));

		// - max single component value
		EXPECT_EQ(static_cast<ReceiptType>(0xF000), MakeReceiptType(0xFF, 0, 0));
		EXPECT_EQ(static_cast<ReceiptType>(0x00FF), MakeReceiptType(0, 0xFF, 0));
		EXPECT_EQ(static_cast<ReceiptType>(0x0F00), MakeReceiptType(0, 0, 0xFF));

		// - all component values
		EXPECT_EQ(static_cast<ReceiptType>(0xFFFF), MakeReceiptType(0xFF, 0xFF, 0xFF));
	}

	TEST(TEST_CLASS, CanMakeReceiptTypeViaGenericMacros) {
		// Act:
		DEFINE_RECEIPT_TYPE(BalanceTransfer, Core, Alpha, 0xC);
		DEFINE_RECEIPT_TYPE(BalanceTransfer, Multisig, Beta, 0xD);
		DEFINE_RECEIPT_TYPE(Other, Aggregate, Gamma, 0xE);

		// Assert:
		EXPECT_EQ(static_cast<ReceiptType>(0x1C43), Receipt_Type_Alpha);
		EXPECT_EQ(static_cast<ReceiptType>(0x1D55), Receipt_Type_Beta);
		EXPECT_EQ(static_cast<ReceiptType>(0x0E41), Receipt_Type_Gamma);
	}

	// endregion

	// region insertion operator

	namespace {
		auto ToReceiptType(uint16_t value) {
			return static_cast<ReceiptType>(value);
		}
	}

	TEST(TEST_CLASS, CanOutputWellKnownEnumValues) {
		EXPECT_EQ("Harvest_Fee", test::ToString(Receipt_Type_Harvest_Fee));
		EXPECT_EQ("Inflation", test::ToString(Receipt_Type_Inflation));
		EXPECT_EQ("Transaction_Group", test::ToString(Receipt_Type_Transaction_Group));
		EXPECT_EQ("Address_Alias_Resolution", test::ToString(Receipt_Type_Address_Alias_Resolution));
		EXPECT_EQ("Mosaic_Alias_Resolution", test::ToString(Receipt_Type_Mosaic_Alias_Resolution));
	}

	TEST(TEST_CLASS, CanOutputPluginEnumValues) {
		EXPECT_EQ("LockHash_Expired", test::ToString(ToReceiptType(0x2348)));
		EXPECT_EQ("LockSecret_Created", test::ToString(ToReceiptType(0x3152)));
		EXPECT_EQ("Mosaic_Expired", test::ToString(ToReceiptType(0x414D)));
		EXPECT_EQ("Namespace_Deleted", test::ToString(ToReceiptType(0x424E)));
	}

	TEST(TEST_CLASS, CanOutputUnknownEnumValues) {
		// Arrange:
		DEFINE_RECEIPT_TYPE(BalanceTransfer, Multisig, Alpha, 0xC);
		DEFINE_RECEIPT_TYPE(BalanceCredit, Core, Beta, 0xD);
		DEFINE_RECEIPT_TYPE(Other, Aggregate, Gamma, 0xE);

		// Assert:
		EXPECT_EQ("ReceiptType<0x1C55>", test::ToString(Receipt_Type_Alpha));
		EXPECT_EQ("ReceiptType<0x2D43>", test::ToString(Receipt_Type_Beta));
		EXPECT_EQ("ReceiptType<0x0E41>", test::ToString(Receipt_Type_Gamma));
	}

	// endregion
}}
