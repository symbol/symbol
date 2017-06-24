#pragma once
#include <iosfwd>

namespace catapult { namespace ionet {

/// Enumeration of socket operation results.
#define SOCKET_OPERATION_CODE_LIST \
	/* The socket operation succeeded. */ \
	ENUM_VALUE(Success) \
	\
	/* The socket was closed. */ \
	ENUM_VALUE(Closed) \
	\
	/* The socket produced a read error. */ \
	ENUM_VALUE(Read_Error) \
	\
	/* The socket produced a write error. */ \
	ENUM_VALUE(Write_Error) \
	\
	/* The socket produced malformed data. */ \
	ENUM_VALUE(Malformed_Data) \
	\
	/* The socket operation completed due to insufficient data. */ \
	ENUM_VALUE(Insufficient_Data)

#define DECLARE_ENUM SocketOperationCode
#define ENUM_LIST SOCKET_OPERATION_CODE_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef DECLARE_ENUM
}}
