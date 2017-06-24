#include "PeerConnectResult.h"
#include "catapult/utils/MacroBasedEnumIncludes.h"

namespace catapult { namespace net {

#define DEFINE_ENUM PeerConnectResult
#define ENUM_LIST PEER_CONNECT_RESULT_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef DEFINE_ENUM
}}
