#pragma once
#include <iosfwd>

namespace catapult { namespace ionet {

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

#define ENUM_VALUE(LABEL) LABEL,
	/// Enumeration of socket operation results.
	enum class SocketOperationCode {
		SOCKET_OPERATION_CODE_LIST
	};
#undef ENUM_VALUE

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, SocketOperationCode value);
}}
