#pragma once
#include "catapult/functions.h"
#include "catapult/types.h"
#include <memory>

namespace catapult {
	namespace crypto { class KeyPair; }
	namespace ionet { class PacketIo; }
}

namespace catapult { namespace net {

#define VERIFY_RESULT_LIST \
	/* There was an i/o error while processing a server challenge request. */ \
	ENUM_VALUE(Io_Error_ServerChallengeRequest) \
	\
	/* There was an i/o error while processing a server challenge response. */ \
	ENUM_VALUE(Io_Error_ServerChallengeResponse) \
	\
	/* There was an i/o error while processing a client challenge response. */ \
	ENUM_VALUE(Io_Error_ClientChallengeResponse) \
	\
	/* The peer sent malformed data. */ \
	ENUM_VALUE(Malformed_Data) \
	\
	/* The peer failed the challenge. */ \
	ENUM_VALUE(Failure_Challenge) \
	\
	/* The peer passed the challenge. */ \
	ENUM_VALUE(Success) \

#define ENUM_VALUE(LABEL) LABEL,
	/// Enumeration of verification results.
	enum class VerifyResult {
		VERIFY_RESULT_LIST
	};
#undef ENUM_VALUE

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, VerifyResult value);

	/// Callback that is called with the result of a verify operation and the public key of the remote node on success.
	using VerifyCallback = consumer<VerifyResult, const Key&>;

	/// Attempts to verify a client (\a pClientIo) and calls \a callback on completion. \a keyPair used for
	/// responses from the server.
	void VerifyClient(const std::shared_ptr<ionet::PacketIo>& pClientIo, const crypto::KeyPair& keyPair, const VerifyCallback& callback);

	/// Attempts to verify a server (\a pServerIo) with public key \a serverPublicKey and calls \a callback on
	/// completion. \a keyPair used used for responses from the client.
	void VerifyServer(
			const std::shared_ptr<ionet::PacketIo>& pServerIo,
			const Key& serverPublicKey,
			const crypto::KeyPair& keyPair,
			const VerifyCallback& callback);
}}
