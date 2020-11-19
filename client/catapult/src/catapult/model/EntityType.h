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

#pragma once
#include "FacilityCode.h"
#include "catapult/utils/Casting.h"
#include <iosfwd>
#include <stdint.h>

namespace catapult { namespace model {

	/// Enumeration of basic entity types.
	/// \note BasicEntityType is used as highest bits of entity type.
	enum class BasicEntityType : uint8_t {
		/// Some other type.
		Other = 0x0,

		/// Transaction type.
		Transaction = 0x1,

		/// Block type.
		Block = 0x2
	};

	/// Enumeration of entity types.
	enum class EntityType : uint16_t {};

	/// Makes entity type given \a basicEntityType, \a facilityCode and \a code.
	constexpr EntityType MakeEntityType(BasicEntityType basicEntityType, FacilityCode facilityCode, uint8_t code) {
		return static_cast<EntityType>(
				(static_cast<uint16_t>(basicEntityType) & 0x03) << 14 // 01..02: type
				| ((code & 0xF) << 8) //                                 05..08: code
				| (static_cast<uint8_t>(facilityCode) & 0xFF)); //       09..16: facility
	}

/// Defines entity type given \a BASIC_TYPE, \a FACILITY, \a DESCRIPTION and \a CODE.
#define DEFINE_ENTITY_TYPE(BASIC_TYPE, FACILITY, DESCRIPTION, CODE) \
	constexpr auto Entity_Type_##DESCRIPTION = model::MakeEntityType( \
			(model::BasicEntityType::BASIC_TYPE), \
			(model::FacilityCode::FACILITY), \
			CODE)

	/// Nemesis block.
	DEFINE_ENTITY_TYPE(Block, Core, Block_Nemesis, 0);

	/// Normal block.
	DEFINE_ENTITY_TYPE(Block, Core, Block_Normal, 1);

	/// Importance block.
	DEFINE_ENTITY_TYPE(Block, Core, Block_Importance, 2);

/// Defines transaction type given \a FACILITY, \a DESCRIPTION and \a CODE.
#define DEFINE_TRANSACTION_TYPE(FACILITY, DESCRIPTION, CODE) DEFINE_ENTITY_TYPE(Transaction, FACILITY, DESCRIPTION, CODE)

	/// Voting key link transaction.
	DEFINE_TRANSACTION_TYPE(Core, Voting_Key_Link, 0x01);

	/// Vrf key link transaction.
	DEFINE_TRANSACTION_TYPE(Core, Vrf_Key_Link, 0x02);

	/// Converts an entity \a type into a basic entity type.
	constexpr BasicEntityType ToBasicEntityType(EntityType type) {
		// - 0x8000: block bit
		// - 0x4000: transaction bit
		return 0x8000 == (utils::to_underlying_type(type) & 0xC000)
				? BasicEntityType::Block
				: 0x4000 == (utils::to_underlying_type(type) & 0xC000) ? BasicEntityType::Transaction : BasicEntityType::Other;
	}

	/// Insertion operator for outputting \a entityType to \a out.
	std::ostream& operator<<(std::ostream& out, EntityType entityType);
}}
