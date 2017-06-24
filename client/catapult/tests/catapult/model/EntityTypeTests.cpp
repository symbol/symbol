#include "catapult/model/EntityType.h"
#include "catapult/utils/HexFormatter.h"
#include "catapult/preprocessor.h"
#include "tests/TestHarness.h"

namespace catapult { namespace model {

	namespace {
		EntityType BitwiseOr(EntityType lhs, EntityType rhs) {
			return static_cast<EntityType>(utils::to_underlying_type(lhs) | utils::to_underlying_type(rhs));
		}
	}

	// region ToBasicEntityType

	TEST(EntityTypeTests, CanConvertKnownEntityTypeToBasicEntityType) {
		// Assert:
		// - blocks
		EXPECT_EQ(BasicEntityType::Block, ToBasicEntityType(EntityType::Nemesis_Block));
		EXPECT_EQ(BasicEntityType::Block, ToBasicEntityType(EntityType::Block));

		// - transactions
#define ENUM_VALUE(LABEL, VALUE) EXPECT_EQ(BasicEntityType::Transaction, ToBasicEntityType(EntityType::LABEL));
		TRANSACTION_TYPE_LIST
#undef ENUM_VALUE
	}

	TEST(EntityTypeTests, CanConvertUnknownEntityTypeToBasicEntityType) {
		// Assert:
		// - no special bits are set or multiple special bits are set
		for (auto type : { 0x000A, 0x3000, 0x3FFF, 0x5000, 0x6000, 0x6FFF, 0x9000, 0xC000, 0xFFFF })
			EXPECT_EQ(BasicEntityType::Other, ToBasicEntityType(static_cast<EntityType>(type))) << utils::HexFormat(type);

		// - block bit is set
		for (auto type : { 0x8000, 0x8FFF })
			EXPECT_EQ(BasicEntityType::Block, ToBasicEntityType(static_cast<EntityType>(type))) << utils::HexFormat(type);

		// - transaction bit is set
		for (auto type : { 0x4000, 0x4FFF })
			EXPECT_EQ(BasicEntityType::Transaction, ToBasicEntityType(static_cast<EntityType>(type))) << utils::HexFormat(type);

		EXPECT_EQ(BasicEntityType::Transaction, ToBasicEntityType(BitwiseOr(EntityType::Transfer, EntityType::Register_Namespace)));
	}

	// endregion
}}
