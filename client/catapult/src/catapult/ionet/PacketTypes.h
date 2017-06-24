#pragma once
#include <iosfwd>
#include <stdint.h>

namespace catapult { namespace ionet {

/// An enumeration of known packet types.
#define PACKET_TYPE_LIST \
	/* An undefined packet type. */ \
	ENUM_VALUE(Undefined, 0u) \
	\
	/* A challenge from a server to a client. */ \
	ENUM_VALUE(Server_Challenge, 1u) \
	\
	/* A challenge from a client to a server. */ \
	ENUM_VALUE(Client_Challenge, 2u) \
	\
	/* Blocks have been pushed by a peer. */ \
	ENUM_VALUE(Push_Block, 3u) \
	\
	/* A block has been requested by a peer. */ \
	ENUM_VALUE(Pull_Block, 4u) \
	\
	/* Chain information has been requested by a peer. */ \
	ENUM_VALUE(Chain_Info, 5u) \
	\
	/* Block hashes have been requested by a peer. */ \
	ENUM_VALUE(Block_Hashes, 7u) \
	\
	/* Blocks have been requested by a peer. */ \
	ENUM_VALUE(Pull_Blocks, 8u) \
	\
	/* Transactions have been pushed by an api-node or a peer. */ \
	ENUM_VALUE(Push_Transactions, 9u) \
	\
	/* Unconfirmed transactions have been requested by a peer. */ \
	ENUM_VALUE(Pull_Transactions, 10u) \
	\
	/* Request for the current diagnostic counter values. */ \
	ENUM_VALUE(Diagnostic_Counters, 1000u) \
	\
	/* Account infos have been requested by a client. */ \
	ENUM_VALUE(Account_Infos, 1001u) \
	\
	/* Request from a client to confirm timestamped hashes. */ \
	ENUM_VALUE(Confirm_Timestamped_Hashes, 1002u) \
	\
	/* Namespace infos have been requested by a client. */ \
	ENUM_VALUE(Namespace_Infos, 1003u) \
	\
	/* Mosaic infos have been requested by a client. */ \
	ENUM_VALUE(Mosaic_Infos, 1004u) \

#define DECLARE_ENUM PacketType
#define EXPLICIT_VALUE_ENUM
#define EXPLICIT_TYPE_ENUM uint32_t
#define ENUM_LIST PACKET_TYPE_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef EXPLICIT_TYPE_ENUM
#undef EXPLICIT_VALUE_ENUM
#undef DECLARE_ENUM
}}
