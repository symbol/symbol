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

#include "EntityType.h"
#include "catapult/utils/MacroBasedEnumIncludes.h"

namespace catapult { namespace model {

#define DEFINE_CASE(RESULT) case utils::to_underlying_type(RESULT)

#define CASE_WELL_KNOWN_ENTITY_TYPE(NAME) DEFINE_CASE(Entity_Type_##NAME): return #NAME

#define CUSTOM_ENTITY_TYPE_DEFINITION 1
#undef DEFINE_ENTITY_TYPE

#define STR(SYMBOL) #SYMBOL

#define DEFINE_ENTITY_TYPE(BASIC_TYPE, FACILITY, DESCRIPTION, CODE) \
	DEFINE_CASE(MakeEntityType((model::BasicEntityType::BASIC_TYPE), (model::FacilityCode::FACILITY), CODE)): \
		return STR(DESCRIPTION)

	namespace {
		const char* ToString(EntityType entityType) {
			switch (utils::to_underlying_type(entityType)) {
			// well known types defined in EntityType.h
			CASE_WELL_KNOWN_ENTITY_TYPE(Block_Nemesis);
			CASE_WELL_KNOWN_ENTITY_TYPE(Block_Normal);
			CASE_WELL_KNOWN_ENTITY_TYPE(Block_Importance);

			CASE_WELL_KNOWN_ENTITY_TYPE(Voting_Key_Link);
			CASE_WELL_KNOWN_ENTITY_TYPE(Vrf_Key_Link);

			// plugin entity types
			#include "plugins/txes/account_link/src/model/AccountLinkEntityType.h"
			#include "plugins/txes/aggregate/src/model/AggregateEntityType.h"
			#include "plugins/txes/lock_hash/src/model/HashLockEntityType.h"
			#include "plugins/txes/lock_secret/src/model/SecretLockEntityType.h"
			#include "plugins/txes/metadata/src/model/MetadataEntityType.h"
			#include "plugins/txes/mosaic/src/model/MosaicEntityType.h"
			#include "plugins/txes/multisig/src/model/MultisigEntityType.h"
			#include "plugins/txes/namespace/src/model/NamespaceEntityType.h"
			#include "plugins/txes/restriction_account/src/model/AccountRestrictionEntityType.h"
			#include "plugins/txes/restriction_mosaic/src/model/MosaicRestrictionEntityType.h"
			#include "plugins/txes/transfer/src/model/TransferEntityType.h"
			}

			return nullptr;
		}
	}

	std::ostream& operator<<(std::ostream& out, EntityType entityType) {
		auto pStr = ToString(entityType);
		if (pStr)
			out << pStr;
		else
			out << "EntityType<0x" << utils::HexFormat(utils::to_underlying_type(entityType)) << ">";

		return out;
	}
}}
