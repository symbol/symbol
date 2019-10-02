/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#pragma once
#include "catapult/model/FacilityCode.h"
#include <iosfwd>
#include <stdint.h>

namespace catapult { namespace ionet {

#define FACILITY_BASED_CODE(BASE_VALUE, FACILITY_CODE) (BASE_VALUE + static_cast<uint8_t>(model::FacilityCode::FACILITY_CODE))

#define PACKET_TYPE_LIST \
	/* Undefined packet type. */ \
	ENUM_VALUE(Undefined, 0) \
	\
	/* p2p packets have types [1, 500) */ \
	\
	/* Challenge from a server to a client. */ \
	ENUM_VALUE(Server_Challenge, 1) \
	\
	/* Challenge from a client to a server. */ \
	ENUM_VALUE(Client_Challenge, 2) \
	\
	/* Blocks have been pushed by a peer. */ \
	ENUM_VALUE(Push_Block, 3) \
	\
	/* Block has been requested by a peer. */ \
	ENUM_VALUE(Pull_Block, 4) \
	\
	/* Chain information has been requested by a peer. */ \
	ENUM_VALUE(Chain_Info, 5) \
	\
	/* Block hashes have been requested by a peer. */ \
	ENUM_VALUE(Block_Hashes, 7) \
	\
	/* Blocks have been requested by a peer. */ \
	ENUM_VALUE(Pull_Blocks, 8) \
	\
	/* Transactions have been pushed by an api-node or a peer. */ \
	ENUM_VALUE(Push_Transactions, 9) \
	\
	/* Unconfirmed transactions have been requested by a peer. */ \
	ENUM_VALUE(Pull_Transactions, 10) \
	\
	/* Secure packet with a signature. */ \
	ENUM_VALUE(Secure_Signed, 11) \
	\
	/* Sub cache merkle roots have been requested. */ \
	ENUM_VALUE(Sub_Cache_Merkle_Roots, 12) \
	\
	/* api only packets have types [500, 600) */ \
	\
	/* Partial aggregate transactions have been pushed by an api-node. */ \
	ENUM_VALUE(Push_Partial_Transactions, 500) \
	\
	/* Detached cosignatures have been pushed by an api-node. */ \
	ENUM_VALUE(Push_Detached_Cosignatures, 501) \
	\
	/* Partial transaction infos have been requested by an api-node. */ \
	ENUM_VALUE(Pull_Partial_Transaction_Infos, 502) \
	\
	/* node discovery packets have types [600, 700) */ \
	\
	/* Node information has been pushed by a peer. */ \
	ENUM_VALUE(Node_Discovery_Push_Ping, 600) \
	\
	/* Node information has been requested by a peer. */ \
	ENUM_VALUE(Node_Discovery_Pull_Ping, 601) \
	\
	/* Peers information has been pushed by a peer. */ \
	ENUM_VALUE(Node_Discovery_Push_Peers, 602) \
	\
	/* Peers information has been requested by a peer. */ \
	ENUM_VALUE(Node_Discovery_Pull_Peers, 603) \
	\
	/* time sync packets have types [700, 710) */ \
	\
	/* Network time information has been requested by a peer. */ \
	ENUM_VALUE(Time_Sync_Network_Time, 700) \
	\
	/* state path packets have types [800, 1100) - ordered by facility code name */ \
	\
	/* Account state path has been requested by a client. */ \
	ENUM_VALUE(Account_State_Path, FACILITY_BASED_CODE(800, Core)) \
	\
	/* Hash lock state path has been requested by a client. */ \
	ENUM_VALUE(Hash_Lock_State_Path, FACILITY_BASED_CODE(800, LockHash)) \
	\
	/* Secret lock state path has been requested by a client. */ \
	ENUM_VALUE(Secret_Lock_State_Path, FACILITY_BASED_CODE(800, LockSecret)) \
	\
	/* Metadata state path has been requested by a client. */ \
	ENUM_VALUE(Metadata_State_Path, FACILITY_BASED_CODE(800, Metadata)) \
	\
	/* Mosaic state path has been requested by a client. */ \
	ENUM_VALUE(Mosaic_State_Path, FACILITY_BASED_CODE(800, Mosaic)) \
	\
	/* Multisig state path has been requested by a client. */ \
	ENUM_VALUE(Multisig_State_Path, FACILITY_BASED_CODE(800, Multisig)) \
	\
	/* Namespace state path has been requested by a client. */ \
	ENUM_VALUE(Namespace_State_Path, FACILITY_BASED_CODE(800, Namespace)) \
	\
	/* Account restrictions state path has been requested by a client. */ \
	ENUM_VALUE(Account_Restrictions_State_Path, FACILITY_BASED_CODE(800, RestrictionAccount)) \
	\
	/* Mosaic restrictions state path has been requested by a client. */ \
	ENUM_VALUE(Mosaic_Restrictions_State_Path, FACILITY_BASED_CODE(800, RestrictionMosaic)) \
	\
	/* diagnostic packets have types [1100, 2000) - ordered by facility code name */ \
	\
	/* Request for the current diagnostic counter values. */ \
	ENUM_VALUE(Diagnostic_Counters, 1100) \
	\
	/* Request from a client to confirm timestamped hashes. */ \
	ENUM_VALUE(Confirm_Timestamped_Hashes, 1101) \
	\
	/* Node infos for active nodes have been requested. */ \
	ENUM_VALUE(Active_Node_Infos, 1102) \
	\
	/* Block statement has been requested by a client. */ \
	ENUM_VALUE(Block_Statement, 1103) \
	\
	/* Unlocked accounts have been requested by a client. */ \
	ENUM_VALUE(Unlocked_Accounts, 1104) \
	\
	/* Account infos have been requested by a client. */ \
	ENUM_VALUE(Account_Infos, FACILITY_BASED_CODE(1200, Core)) \
	\
	/* Hash lock infos have been requested by a client. */ \
	ENUM_VALUE(Hash_Lock_Infos, FACILITY_BASED_CODE(1200, LockHash)) \
	\
	/* Secret lock infos have been requested by a client. */ \
	ENUM_VALUE(Secret_Lock_Infos, FACILITY_BASED_CODE(1200, LockSecret)) \
	\
	/* Metadata infos have been requested by a client. */ \
	ENUM_VALUE(Metadata_Infos, FACILITY_BASED_CODE(1200, Metadata)) \
	\
	/* Mosaic infos have been requested by a client. */ \
	ENUM_VALUE(Mosaic_Infos, FACILITY_BASED_CODE(1200, Mosaic)) \
	\
	/* Multisig infos have been requested by a client. */ \
	ENUM_VALUE(Multisig_Infos, FACILITY_BASED_CODE(1200, Multisig)) \
	\
	/* Namespace infos have been requested by a client. */ \
	ENUM_VALUE(Namespace_Infos, FACILITY_BASED_CODE(1200, Namespace)) \
	\
	/* Account restrictions infos have been requested by a client. */ \
	ENUM_VALUE(Account_Restrictions_Infos, FACILITY_BASED_CODE(1200, RestrictionAccount)) \
	\
	/* Mosaic restrictions infos have been requested by a client. */ \
	ENUM_VALUE(Mosaic_Restrictions_Infos, FACILITY_BASED_CODE(1200, RestrictionMosaic))

#define ENUM_VALUE(LABEL, VALUE) LABEL = VALUE,
	/// Enumeration of known packet types.
	enum class PacketType : uint32_t {
		PACKET_TYPE_LIST
	};
#undef ENUM_VALUE

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, PacketType value);
}}
