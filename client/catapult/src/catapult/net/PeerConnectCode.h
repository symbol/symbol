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
#include <iosfwd>

namespace catapult { namespace net {

#define PEER_CONNECT_CODE_LIST \
	/* Underlying socket operation failed. */ \
	ENUM_VALUE(Socket_Error) \
	\
	/* Peer failed verification. */ \
	ENUM_VALUE(Verify_Error) \
	\
	/* Self connection was detected and bypassed. */ \
	ENUM_VALUE(Self_Connection_Error) \
	\
	/* Verification timed out. */ \
	ENUM_VALUE(Timed_Out) \
	\
	/* Peer was accepted. */ \
	ENUM_VALUE(Accepted) \
	\
	/* Peer is already connected. */ \
	ENUM_VALUE(Already_Connected)

#define ENUM_VALUE(LABEL) LABEL,
	/// Enumeration of possible peer connection codes.
	enum class PeerConnectCode {
		PEER_CONNECT_CODE_LIST
	};
#undef ENUM_VALUE

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, PeerConnectCode value);
}}
