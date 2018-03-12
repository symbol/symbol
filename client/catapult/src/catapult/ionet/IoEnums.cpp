#include "ConnectResult.h"
#include "PacketExtractor.h"
#include "PacketType.h"
#include "SocketOperationCode.h"
#include "catapult/utils/MacroBasedEnumIncludes.h"

namespace catapult { namespace ionet {

#define DEFINE_ENUM ConnectResult
#define ENUM_LIST CONNECT_RESULT_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef DEFINE_ENUM

#define DEFINE_ENUM PacketExtractResult
#define ENUM_LIST PACKET_EXTRACT_RESULT_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef DEFINE_ENUM

#define DEFINE_ENUM SocketOperationCode
#define ENUM_LIST SOCKET_OPERATION_CODE_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef DEFINE_ENUM

#define DEFINE_ENUM PacketType
#define EXPLICIT_VALUE_ENUM
#define ENUM_LIST PACKET_TYPE_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef EXPLICIT_VALUE_ENUM
#undef DEFINE_ENUM

}}
