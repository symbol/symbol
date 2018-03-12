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
			CASE_WELL_KNOWN_ENTITY_TYPE(Nemesis_Block);
			CASE_WELL_KNOWN_ENTITY_TYPE(Block);

			// plugin entity types
			#include "plugins/txes/aggregate/src/model/AggregateEntityType.h"
			#include "plugins/txes/multisig/src/model/MultisigEntityType.h"
			#include "plugins/txes/namespace/src/model/MosaicEntityType.h"
			#include "plugins/txes/namespace/src/model/NamespaceEntityType.h"
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
			out << "EntityType(0x" << utils::HexFormat(utils::to_underlying_type(entityType)) << ")";

		return out;
	}
}}
