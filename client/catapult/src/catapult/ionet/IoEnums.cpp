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

#include "ConnectResult.h"
#include "PacketExtractor.h"
#include "PacketType.h"
#include "SocketOperationCode.h"
#include "catapult/utils/ConfigurationValueParsers.h"
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
