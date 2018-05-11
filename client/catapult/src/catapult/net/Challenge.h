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
#include "catapult/ionet/Packet.h"
#include "catapult/ionet/PacketHandlers.h"
#include "catapult/types.h"

namespace catapult { namespace crypto { class KeyPair; } }

namespace catapult { namespace net {

	/// Challenge data.
	using Challenge = std::array<uint8_t, 64>;

#pragma pack(push, 1)

	/// Packet representing a challenge request from a server to a client.
	struct ServerChallengeRequest : public ionet::Packet {
		static constexpr ionet::PacketType Packet_Type = ionet::PacketType::Server_Challenge;

		/// Challenge data that should be signed by the client.
		net::Challenge Challenge;
	};

	/// Packet representing a challenge response and new challenge request from a client to a server.
	struct ServerChallengeResponse : public ionet::Packet {
		static constexpr ionet::PacketType Packet_Type = ionet::PacketType::Server_Challenge;

		/// Challenge data that should be signed by the server.
		net::Challenge Challenge;

		/// Client's signature on the server challenge and any additional request information.
		catapult::Signature Signature;

		/// Client's public key.
		Key PublicKey;

		/// Security mode requested by the client.
		ionet::ConnectionSecurityMode SecurityMode;
	};

	/// Packet representing a challenge response from a server to a client.
	struct ClientChallengeResponse : public ionet::Packet {
		static constexpr ionet::PacketType Packet_Type = ionet::PacketType::Client_Challenge;

		/// Server's signature on the client challenge.
		catapult::Signature Signature;
	};

#pragma pack(pop)

	/// Generates a random server challenge request that is sent to a client.
	std::shared_ptr<ServerChallengeRequest> GenerateServerChallengeRequest();

	/// Generates a client response to a server challenge (\a request) using the client key pair (\a keyPair)
	/// and requests the specified security mode (\a securityMode).
	std::shared_ptr<ServerChallengeResponse> GenerateServerChallengeResponse(
			const ServerChallengeRequest& request,
			const crypto::KeyPair& keyPair,
			ionet::ConnectionSecurityMode securityMode);

	/// Verifies a client's \a response to \a challenge.
	bool VerifyServerChallengeResponse(const ServerChallengeResponse& response, const Challenge& challenge);

	/// Generates a server response to a client challenge (\a request) using the server key pair (\a keyPair).
	std::shared_ptr<ClientChallengeResponse> GenerateClientChallengeResponse(
			const ServerChallengeResponse& request,
			const crypto::KeyPair& keyPair);

	/// Verifies a server's \a response to \a challenge assuming the server has a public key
	/// of \a serverPublicKey.
	bool VerifyClientChallengeResponse(const ClientChallengeResponse& response, const Key& serverPublicKey, const Challenge& challenge);
}}
