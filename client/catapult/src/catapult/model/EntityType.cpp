#include "EntityType.h"
#include "catapult/utils/MacroBasedEnumIncludes.h"

namespace catapult { namespace model {

#define DEFINE_ENUM EntityType
#define EXPLICIT_VALUE_ENUM
#define ENUM_LIST ENTITY_TYPE_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef EXPLICIT_VALUE_ENUM
#undef DEFINE_ENUM

#define DEFINE_ENUM BasicEntityType
#define ENUM_LIST BASIC_ENTITY_TYPE_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef DEFINE_ENUM
}}
