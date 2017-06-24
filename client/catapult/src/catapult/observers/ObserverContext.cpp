#include "ObserverContext.h"
#include "catapult/utils/MacroBasedEnumIncludes.h"

namespace catapult { namespace observers {

#define DEFINE_ENUM NotifyMode
#define ENUM_LIST NOTIFY_MODE_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef DEFINE_ENUM
}}
