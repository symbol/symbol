#pragma once
#include "catapult/types.h"

namespace catapult { namespace crypto { class PrivateKey; } }

namespace catapult { namespace crypto {

	/// Extracts a public key (\a publicKey) from a private key (\a)
	void ExtractPublicKeyFromPrivateKey(const PrivateKey& privateKey, Key& publicKey);
}}
