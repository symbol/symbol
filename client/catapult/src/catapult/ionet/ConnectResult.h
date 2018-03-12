#pragma once
#include <iosfwd>

namespace catapult { namespace ionet {

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

#define ENUM_VALUE(LABEL) LABEL,
	/// Enumeration of possible connection results.
	enum class ConnectResult {
		CONNECT_RESULT_LIST
	};
#undef ENUM_VALUE

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, ConnectResult value);
}}
