#pragma once
#include "catapult/ionet/Packet.h"
#include "catapult/ionet/PacketHandlers.h"
#include "catapult/types.h"

namespace catapult { namespace crypto { class KeyPair; } }

namespace catapult { namespace net {

	const size_t Challenge_Size = 64;
	using Challenge = std::array<uint8_t, Challenge_Size>;

#pragma pack(push, 1)

	/// A packet representing a challenge request from a server to a client.
	struct ServerChallengeRequest : public ionet::Packet {
		static constexpr ionet::PacketType Packet_Type = ionet::PacketType::Server_Challenge;

		/// The challenge data that should be signed by the client.
		net::Challenge Challenge;
	};

	/// A packet representing a challenge response from a client to a server.
	struct ServerChallengeResponse : public ionet::Packet {
		static constexpr ionet::PacketType Packet_Type = ionet::PacketType::Server_Challenge;

		/// The challenge data that should be signed by the server.
		net::Challenge Challenge;

		/// The client's signature on the server challenge.
		catapult::Signature Signature;

		/// The client's public key.
		Key PublicKey;
	};

	using ClientChallengeRequest = ServerChallengeResponse;

	/// A packet representing a challenge response from a server to a client.
	struct ClientChallengeResponse : public ionet::Packet {
		static constexpr ionet::PacketType Packet_Type = ionet::PacketType::Client_Challenge;

		/// The server's signature on the client challenge.
		catapult::Signature Signature;
	};

#pragma pack(pop)

	/// Generates a random server challenge request that is sent to a client.
	std::shared_ptr<ServerChallengeRequest> GenerateServerChallengeRequest();

	/// Generates a client response to a server challenge (\a request) using the client key pair (\a keyPair).
	std::shared_ptr<ServerChallengeResponse> GenerateServerChallengeResponse(
			const ServerChallengeRequest& request,
			const crypto::KeyPair& keyPair);

	/// Verifies a client's \a response to \a challenge.
	bool VerifyServerChallengeResponse(const ServerChallengeResponse& response, const Challenge& challenge);

	/// Generates a server response to a client challenge (\a request) using the server key pair (\a keyPair).
	std::shared_ptr<ClientChallengeResponse> GenerateClientChallengeResponse(
			const ClientChallengeRequest& request,
			const crypto::KeyPair& keyPair);

	/// Verifies a server's \a response to \a challenge assuming the server has a public key
	/// of \a serverPublicKey.
	bool VerifyClientChallengeResponse(const ClientChallengeResponse& response, const Key& serverPublicKey, const Challenge& challenge);
}}
