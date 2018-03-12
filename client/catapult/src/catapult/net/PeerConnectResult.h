#pragma once
#include <iosfwd>

namespace catapult { namespace net {

#define PEER_CONNECT_RESULT_LIST \
	/* The underlying socket operation failed. */ \
	ENUM_VALUE(Socket_Error) \
	\
	/* The peer failed verification. */ \
	ENUM_VALUE(Verify_Error) \
	\
	/* The verification timed out. */ \
	ENUM_VALUE(Timed_Out) \
	\
	/* The peer was accepted. */ \
	ENUM_VALUE(Accepted) \
	\
	/* The peer is already connected. */ \
	ENUM_VALUE(Already_Connected) \

#define ENUM_VALUE(LABEL) LABEL,
	/// Enumeration of possible peer connection results.
	enum class PeerConnectResult {
		PEER_CONNECT_RESULT_LIST
	};
#undef ENUM_VALUE

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, PeerConnectResult value);
}}
