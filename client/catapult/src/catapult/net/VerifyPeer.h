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
#include "catapult/ionet/ConnectionSecurityMode.h"
#include "catapult/functions.h"
#include "catapult/types.h"
#include <memory>

namespace catapult {
	namespace crypto { class KeyPair; }
	namespace ionet { class PacketIo; }
}

namespace catapult { namespace net {

#define VERIFY_RESULT_LIST \
	/* An i/o error occurred while processing a server challenge request. */ \
	ENUM_VALUE(Io_Error_ServerChallengeRequest) \
	\
	/* An i/o error occurred while processing a server challenge response. */ \
	ENUM_VALUE(Io_Error_ServerChallengeResponse) \
	\
	/* An i/o error occurred while processing a client challenge response. */ \
	ENUM_VALUE(Io_Error_ClientChallengeResponse) \
	\
	/* Peer sent malformed data. */ \
	ENUM_VALUE(Malformed_Data) \
	\
	/* Peer failed the challenge. */ \
	ENUM_VALUE(Failure_Challenge) \
	\
	/* Peer requested an unsupported connection (e.g. unsupported security mode). */ \
	ENUM_VALUE(Failure_Unsupported_Connection) \
	\
	/* Peer passed the challenge. */ \
	ENUM_VALUE(Success)

#define ENUM_VALUE(LABEL) LABEL,
	/// Enumeration of verification results.
	enum class VerifyResult {
		VERIFY_RESULT_LIST
	};
#undef ENUM_VALUE

	/// Information about the verified node.
	struct VerifiedPeerInfo {
		/// Public key of the node.
		Key PublicKey;

		/// Security mode established.
		ionet::ConnectionSecurityMode SecurityMode;
	};

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, VerifyResult value);

	/// Callback that is called with the result of a verify operation and verified peer information on success.
	using VerifyCallback = consumer<VerifyResult, const VerifiedPeerInfo&>;

	/// Attempts to verify a client (\a pClientIo) and calls \a callback on completion.
	/// Only security modes set in \a allowedSecurityModes are allowed.
	/// \a keyPair is used for responses from the server.
	void VerifyClient(
			const std::shared_ptr<ionet::PacketIo>& pClientIo,
			const crypto::KeyPair& keyPair,
			ionet::ConnectionSecurityMode allowedSecurityModes,
			const VerifyCallback& callback);

	/// Attempts to verify a server (\a pServerIo) using \a serverPeerInfo and calls \a callback on completion.
	/// \a keyPair is used for responses from the client.
	void VerifyServer(
			const std::shared_ptr<ionet::PacketIo>& pServerIo,
			const VerifiedPeerInfo& serverPeerInfo,
			const crypto::KeyPair& keyPair,
			const VerifyCallback& callback);
}}
