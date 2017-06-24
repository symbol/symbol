#include "InputSource.h"
#include "catapult/utils/MacroBasedEnumIncludes.h"

namespace catapult { namespace disruptor {

#define DEFINE_ENUM InputSource
#define EXPLICIT_VALUE_ENUM
#define ENUM_LIST INPUT_SOURCE_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef EXPLICIT_VALUE_ENUM
#undef DEFINE_ENUM
}}
