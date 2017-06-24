#pragma once
#include "catapult/utils/Casting.h"
#include <iosfwd>
#include <stdint.h>

namespace catapult { namespace model {

/// Enumeration of transaction types.
#define TRANSACTION_TYPE_LIST \
/* Transfer transaction. */ \
	ENUM_VALUE(Transfer, 0x4101) \
	/* Register namespace transaction. */ \
	ENUM_VALUE(Register_Namespace, 0x4201) \
	/* Mosaic definition transaction. */ \
	ENUM_VALUE(Mosaic_Definition, 0x4202) \
	/* Mosaic levy change transaction. */ \
	ENUM_VALUE(Mosaic_Levy_Change, 0x4203) \
	/* Mosaic supply change transaction. */ \
	ENUM_VALUE(Mosaic_Supply_Change, 0x4204) \
	/* Modify multisig account transaction */ \
	ENUM_VALUE(Modify_Multisig_Account, 0x4401) \
	/* Aggregate transaction */ \
	ENUM_VALUE(Aggregate, 0x4801)

/// Enumeration of entity types.
#define ENTITY_TYPE_LIST \
	/* Nemesis block. */ \
	ENUM_VALUE(Nemesis_Block, 0x8000) \
	/* Block. */ \
	ENUM_VALUE(Block, 0x8001) \
	\
	TRANSACTION_TYPE_LIST

#define DECLARE_ENUM EntityType
#define EXPLICIT_VALUE_ENUM
#define EXPLICIT_TYPE_ENUM uint16_t
#define ENUM_LIST ENTITY_TYPE_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef EXPLICIT_TYPE_ENUM
#undef EXPLICIT_VALUE_ENUM
#undef DECLARE_ENUM

/// Enumeration of basic entity types.
#define BASIC_ENTITY_TYPE_LIST \
	/* A type of block. */ \
	ENUM_VALUE(Block) \
	/* A type of transaction. */ \
	ENUM_VALUE(Transaction) \
	/* Some other type of entity. */ \
	ENUM_VALUE(Other)

#define DECLARE_ENUM BasicEntityType
#define ENUM_LIST BASIC_ENTITY_TYPE_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef DECLARE_ENUM

	/// Converts an entity \a type into a basic entity type.
	constexpr BasicEntityType ToBasicEntityType(EntityType type) {
		// - 0x8000: block bit
		// - 0x4000: transaction bit
		return 0x8000 == (utils::to_underlying_type(type) & 0xF000)
				? BasicEntityType::Block
				: 0x4000 == (utils::to_underlying_type(type) & 0xF000) ? BasicEntityType::Transaction : BasicEntityType::Other;
	}
}}
