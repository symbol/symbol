#pragma once
#include <iosfwd>

namespace catapult { namespace ionet {

/// Enumeration of possible connection results.
#define CONNECT_RESULT_LIST \
	/* Client address could not be resolved. */ \
	ENUM_VALUE(Resolve_Error) \
	\
	/* Connection could not be established. */ \
	ENUM_VALUE(Connect_Error) \
	\
	/* Connection attempt was cancelled. */ \
	ENUM_VALUE(Connect_Cancelled) \
	\
	/* Connection was successfully established. */ \
	ENUM_VALUE(Connected)

#define DECLARE_ENUM ConnectResult
#define ENUM_LIST CONNECT_RESULT_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef DECLARE_ENUM
}}
