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
	/* p2p packets have types [1, 0x100) */ \
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
	/* Chain statistics have been requested by a peer. */ \
	ENUM_VALUE(Chain_Statistics, 5) \
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
	/* partial transactions packets have types [0x100, 0x110) */ \
	\
	/* Partial aggregate transactions have been pushed by an api-node. */ \
	ENUM_VALUE(Push_Partial_Transactions, 0x100) \
	\
	/* Detached cosignatures have been pushed by an api-node. */ \
	ENUM_VALUE(Push_Detached_Cosignatures, 0x101) \
	\
	/* Partial transaction infos have been requested by an api-node. */ \
	ENUM_VALUE(Pull_Partial_Transaction_Infos, 0x102) \
	\
	/* node discovery packets have types [0x110, 0x120) */ \
	\
	/* Node information has been pushed by a peer. */ \
	ENUM_VALUE(Node_Discovery_Push_Ping, 0x110) \
	\
	/* Node information has been requested by a peer. */ \
	ENUM_VALUE(Node_Discovery_Pull_Ping, 0x111) \
	\
	/* Peers information has been pushed by a peer. */ \
	ENUM_VALUE(Node_Discovery_Push_Peers, 0x112) \
	\
	/* Peers information has been requested by a peer. */ \
	ENUM_VALUE(Node_Discovery_Pull_Peers, 0x113) \
	\
	/* time sync packets have types [0x120, 0x130) */ \
	\
	/* Network time information has been requested by a peer. */ \
	ENUM_VALUE(Time_Sync_Network_Time, 0x120) \
	\
	/* finalization packets have types [0x130, 0x140) */ \
	\
	/* Finalization messages have been pushed by a peer. */ \
	ENUM_VALUE(Push_Finalization_Messages, 0x130) \
	\
	/* Finalization messages have been requested by a peer. */ \
	ENUM_VALUE(Pull_Finalization_Messages, 0x131) \
	\
	/* Finalization statistics have been requested by a peer. */ \
	ENUM_VALUE(Finalization_Statistics, 0x132) \
	\
	/* Finalization proof has been requested at a specific finalization point. */ \
	ENUM_VALUE(Finalization_Proof_At_Point, 0x133) \
	\
	/* Finalization proof has been requested at a specific finalization height. */ \
	ENUM_VALUE(Finalization_Proof_At_Height, 0x134) \
	\
	/* Finalization proof has been requested by a peer. */ \
	ENUM_VALUE(Pull_Finalization_Proof, 0x135) \
	\
	/* state path packets have types [0x200, 0x300) - ordered by facility code name */ \
	\
	/* Account state path has been requested by a client. */ \
	ENUM_VALUE(Account_State_Path, FACILITY_BASED_CODE(0x200, Core)) \
	\
	/* Hash lock state path has been requested by a client. */ \
	ENUM_VALUE(Hash_Lock_State_Path, FACILITY_BASED_CODE(0x200, LockHash)) \
	\
	/* Secret lock state path has been requested by a client. */ \
	ENUM_VALUE(Secret_Lock_State_Path, FACILITY_BASED_CODE(0x200, LockSecret)) \
	\
	/* Metadata state path has been requested by a client. */ \
	ENUM_VALUE(Metadata_State_Path, FACILITY_BASED_CODE(0x200, Metadata)) \
	\
	/* Mosaic state path has been requested by a client. */ \
	ENUM_VALUE(Mosaic_State_Path, FACILITY_BASED_CODE(0x200, Mosaic)) \
	\
	/* Multisig state path has been requested by a client. */ \
	ENUM_VALUE(Multisig_State_Path, FACILITY_BASED_CODE(0x200, Multisig)) \
	\
	/* Namespace state path has been requested by a client. */ \
	ENUM_VALUE(Namespace_State_Path, FACILITY_BASED_CODE(0x200, Namespace)) \
	\
	/* Account restrictions state path has been requested by a client. */ \
	ENUM_VALUE(Account_Restrictions_State_Path, FACILITY_BASED_CODE(0x200, RestrictionAccount)) \
	\
	/* Mosaic restrictions state path has been requested by a client. */ \
	ENUM_VALUE(Mosaic_Restrictions_State_Path, FACILITY_BASED_CODE(0x200, RestrictionMosaic)) \
	\
	/* diagnostic packets have types [0x300, 0x400) */ \
	\
	/* Request for the current diagnostic counter values. */ \
	ENUM_VALUE(Diagnostic_Counters, 0x300) \
	\
	/* Request from a client to confirm timestamped hashes. */ \
	ENUM_VALUE(Confirm_Timestamped_Hashes, 0x301) \
	\
	/* Node infos for active nodes have been requested. */ \
	ENUM_VALUE(Active_Node_Infos, 0x302) \
	\
	/* Block statement has been requested by a client. */ \
	ENUM_VALUE(Block_Statement, 0x303) \
	\
	/* Unlocked accounts have been requested by a client. */ \
	ENUM_VALUE(Unlocked_Accounts, 0x304) \
	\
	/* diagnostic info packets have types [0x400, 0x500) - ordered by facility code name */ \
	\
	/* Account infos have been requested by a client. */ \
	ENUM_VALUE(Account_Infos, FACILITY_BASED_CODE(0x400, Core)) \
	\
	/* Hash lock infos have been requested by a client. */ \
	ENUM_VALUE(Hash_Lock_Infos, FACILITY_BASED_CODE(0x400, LockHash)) \
	\
	/* Secret lock infos have been requested by a client. */ \
	ENUM_VALUE(Secret_Lock_Infos, FACILITY_BASED_CODE(0x400, LockSecret)) \
	\
	/* Metadata infos have been requested by a client. */ \
	ENUM_VALUE(Metadata_Infos, FACILITY_BASED_CODE(0x400, Metadata)) \
	\
	/* Mosaic infos have been requested by a client. */ \
	ENUM_VALUE(Mosaic_Infos, FACILITY_BASED_CODE(0x400, Mosaic)) \
	\
	/* Multisig infos have been requested by a client. */ \
	ENUM_VALUE(Multisig_Infos, FACILITY_BASED_CODE(0x400, Multisig)) \
	\
	/* Namespace infos have been requested by a client. */ \
	ENUM_VALUE(Namespace_Infos, FACILITY_BASED_CODE(0x400, Namespace)) \
	\
	/* Account restrictions infos have been requested by a client. */ \
	ENUM_VALUE(Account_Restrictions_Infos, FACILITY_BASED_CODE(0x400, RestrictionAccount)) \
	\
	/* Mosaic restrictions infos have been requested by a client. */ \
	ENUM_VALUE(Mosaic_Restrictions_Infos, FACILITY_BASED_CODE(0x400, RestrictionMosaic))

#define ENUM_VALUE(LABEL, VALUE) LABEL = VALUE,
	/// Enumeration of known packet types.
	enum class PacketType : uint32_t {
		PACKET_TYPE_LIST
	};
#undef ENUM_VALUE

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, PacketType value);
}}
