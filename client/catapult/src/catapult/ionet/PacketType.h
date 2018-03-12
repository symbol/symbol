#pragma once
#include <iosfwd>
#include <stdint.h>

namespace catapult { namespace ionet {

#define PACKET_TYPE_LIST \
	/* An undefined packet type. */ \
	ENUM_VALUE(Undefined, 0u) \
	\
	/* p2p packets have types [1, 500) */ \
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
	/* api only packets have types [500, 600) */ \
	\
	/* Partial aggregate transactions have been pushed by an api-node. */ \
	ENUM_VALUE(Push_Partial_Transactions, 500u) \
	\
	/* Detached cosignatures have been pushed by an api-node. */ \
	ENUM_VALUE(Push_Detached_Cosignatures, 501u) \
	\
	/* Partial transaction infos have been requested by an api-node. */ \
	ENUM_VALUE(Pull_Partial_Transaction_Infos, 502u) \
	\
	/* node discovery packets have types [600, 700) */ \
	\
	/* Node information has been pushed by a peer. */ \
	ENUM_VALUE(Node_Discovery_Push_Ping, 600u) \
	\
	/* Node information has been requested by a peer. */ \
	ENUM_VALUE(Node_Discovery_Pull_Ping, 601) \
	\
	/* Peers information has been pushed by a peer. */ \
	ENUM_VALUE(Node_Discovery_Push_Peers, 602u) \
	\
	/* Peers information has been requested by a peer. */ \
	ENUM_VALUE(Node_Discovery_Pull_Peers, 603u) \
	\
	/* diagnostic packets have types [1000, 2000) */ \
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
	\
	/* Node infos for active nodes have been requested. */ \
	ENUM_VALUE(Active_Node_Infos, 1005u)

#define ENUM_VALUE(LABEL, VALUE) LABEL = VALUE,
	/// An enumeration of known packet types.
	enum class PacketType : uint32_t {
		PACKET_TYPE_LIST
	};
#undef ENUM_VALUE

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, PacketType value);
}}
