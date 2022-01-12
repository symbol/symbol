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

#include "catapult/model/EntityType.h"
#include "catapult/utils/HexFormatter.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

#define TEST_CLASS EntityTypeTests

	namespace {
		EntityType BitwiseOr(EntityType lhs, EntityType rhs) {
			return static_cast<EntityType>(utils::to_underlying_type(lhs) | utils::to_underlying_type(rhs));
		}

		auto MakeEntityType(uint8_t basicEntityType, uint8_t facility, uint8_t code) {
			return MakeEntityType(static_cast<BasicEntityType>(basicEntityType), static_cast<FacilityCode>(facility), code);
		}
	}

	// region MakeEntityType

	TEST(TEST_CLASS, CanMakeEntityType) {
		// Assert: zeros
		EXPECT_EQ(static_cast<EntityType>(0x00000000), MakeEntityType(0, 0, 0));

		// - max single component value
		EXPECT_EQ(static_cast<EntityType>(0xC000), MakeEntityType(0xFF, 0, 0));
		EXPECT_EQ(static_cast<EntityType>(0x00FF), MakeEntityType(0, 0xFF, 0));
		EXPECT_EQ(static_cast<EntityType>(0x0F00), MakeEntityType(0, 0, 0xFF));

		// - all component values
		EXPECT_EQ(static_cast<EntityType>(0xCFFF), MakeEntityType(0xFF, 0xFF, 0xFF));
	}

	TEST(TEST_CLASS, CanMakeEntityTypeViaGenericMacros) {
		// Act:
		DEFINE_ENTITY_TYPE(Block, Core, Alpha, 0xC);
		DEFINE_ENTITY_TYPE(Block, Multisig, Beta, 0xD);
		DEFINE_ENTITY_TYPE(Other, Aggregate, Gamma, 0xE);

		// Assert:
		EXPECT_EQ(static_cast<EntityType>(0x8C43), Entity_Type_Alpha);
		EXPECT_EQ(static_cast<EntityType>(0x8D55), Entity_Type_Beta);
		EXPECT_EQ(static_cast<EntityType>(0x0E41), Entity_Type_Gamma);
	}

	TEST(TEST_CLASS, CanMakeEntityTypeViaTransactionMacros) {
		// Act:
		DEFINE_TRANSACTION_TYPE(Namespace, Alpha, 0xC);
		DEFINE_TRANSACTION_TYPE(Multisig, Beta, 0xD);

		// Assert:
		EXPECT_EQ(static_cast<EntityType>(0x4C4E), Entity_Type_Alpha);
		EXPECT_EQ(static_cast<EntityType>(0x4D55), Entity_Type_Beta);
	}

	// endregion

	// region ToBasicEntityType

	TEST(TEST_CLASS, CanConvertKnownEntityTypeToBasicEntityType) {
		// Assert: blocks
		EXPECT_EQ(BasicEntityType::Block, ToBasicEntityType(Entity_Type_Block_Nemesis));
		EXPECT_EQ(BasicEntityType::Block, ToBasicEntityType(Entity_Type_Block_Normal));
		EXPECT_EQ(BasicEntityType::Block, ToBasicEntityType(Entity_Type_Block_Importance));

		// - transactions
		EXPECT_EQ(BasicEntityType::Transaction, ToBasicEntityType(Entity_Type_Vrf_Key_Link));
		EXPECT_EQ(BasicEntityType::Transaction, ToBasicEntityType(Entity_Type_Voting_Key_Link));
	}

	TEST(TEST_CLASS, CanConvertUnknownEntityTypeToBasicEntityType) {
		// Arrange:
		DEFINE_TRANSACTION_TYPE(Transfer, Dummy_Transfer, 0xC);
		DEFINE_TRANSACTION_TYPE(Multisig, Dummy_Multisig, 0xD);

		// Sanity:
		EXPECT_NE(Entity_Type_Dummy_Transfer, Entity_Type_Dummy_Multisig);

		// Assert:
		// - no special bits are set or multiple special bits are set
		for (auto type : { 0x000A, 0x3000, 0x3FFF, 0xC000, 0xFFFF })
			EXPECT_EQ(BasicEntityType::Other, ToBasicEntityType(static_cast<EntityType>(type))) << utils::HexFormat(type);

		// - block bit is set
		for (auto type : { 0x8000, 0x8FFF })
			EXPECT_EQ(BasicEntityType::Block, ToBasicEntityType(static_cast<EntityType>(type))) << utils::HexFormat(type);

		// - transaction bit is set
		for (auto type : { 0x4000, 0x4FFF })
			EXPECT_EQ(BasicEntityType::Transaction, ToBasicEntityType(static_cast<EntityType>(type))) << utils::HexFormat(type);

		EXPECT_EQ(BasicEntityType::Transaction, ToBasicEntityType(BitwiseOr(Entity_Type_Dummy_Transfer, Entity_Type_Dummy_Multisig)));
	}

	// endregion

	// region insertion operator

	namespace {
		auto ToEntityType(uint16_t value) {
			return static_cast<EntityType>(value);
		}
	}

	TEST(TEST_CLASS, CanOutputBlockEnumValues) {
		EXPECT_EQ("Block_Nemesis", test::ToString(Entity_Type_Block_Nemesis));
		EXPECT_EQ("Block_Normal", test::ToString(Entity_Type_Block_Normal));
		EXPECT_EQ("Block_Importance", test::ToString(Entity_Type_Block_Importance));
	}

	TEST(TEST_CLASS, CanOutputCoreTransactionEnumValues) {
		EXPECT_EQ("Vrf_Key_Link", test::ToString(Entity_Type_Vrf_Key_Link));
		EXPECT_EQ("Voting_Key_Link", test::ToString(Entity_Type_Voting_Key_Link));
	}

	TEST(TEST_CLASS, CanOutputPluginEnumValues) {
		// Assert: ordered by facility code
		EXPECT_EQ("Aggregate_Complete", test::ToString(ToEntityType(0x4141)));
		EXPECT_EQ("Mosaic_Metadata", test::ToString(ToEntityType(0x4244)));
		EXPECT_EQ("Hash_Lock", test::ToString(ToEntityType(0x4148)));
		EXPECT_EQ("Account_Key_Link", test::ToString(ToEntityType(0x414C)));
		EXPECT_EQ("Mosaic_Supply_Change", test::ToString(ToEntityType(0x424D)));
		EXPECT_EQ("Namespace_Registration", test::ToString(ToEntityType(0x414E)));
		EXPECT_EQ("Account_Mosaic_Restriction", test::ToString(ToEntityType(0x4250)));
		EXPECT_EQ("Mosaic_Global_Restriction", test::ToString(ToEntityType(0x4151)));
		EXPECT_EQ("Secret_Lock", test::ToString(ToEntityType(0x4152)));
		EXPECT_EQ("Transfer", test::ToString(ToEntityType(0x4154)));
		EXPECT_EQ("Multisig_Account_Modification", test::ToString(ToEntityType(0x4155)));
	}

	TEST(TEST_CLASS, CanOutputUnknownEnumValues) {
		// Arrange:
		DEFINE_ENTITY_TYPE(Block, Multisig, Alpha, 0xC);
		DEFINE_ENTITY_TYPE(Transaction, Core, Beta, 0xD);
		DEFINE_ENTITY_TYPE(Other, Aggregate, Gamma, 0xE);

		// Assert:
		EXPECT_EQ("EntityType<0x8C55>", test::ToString(Entity_Type_Alpha));
		EXPECT_EQ("EntityType<0x4D43>", test::ToString(Entity_Type_Beta));
		EXPECT_EQ("EntityType<0x0E41>", test::ToString(Entity_Type_Gamma));
	}

	// endregion
}}
