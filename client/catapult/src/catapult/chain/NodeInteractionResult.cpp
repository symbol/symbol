#include "NodeInteractionResult.h"
#include "catapult/utils/MacroBasedEnumIncludes.h"

namespace catapult { namespace chain {

#define DEFINE_ENUM NodeInteractionResult
#define ENUM_LIST NODE_INTERACTION_RESULT_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef DEFINE_ENUM
}}
