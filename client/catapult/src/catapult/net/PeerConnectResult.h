#pragma once
#include <iosfwd>

namespace catapult { namespace net {

/// Enumeration of possible peer connection results.
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

#define DECLARE_ENUM PeerConnectResult
#define ENUM_LIST PEER_CONNECT_RESULT_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef DECLARE_ENUM
}}
