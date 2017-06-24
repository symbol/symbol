#pragma once
#include "catapult/crypto/KeyPair.h"
#include "catapult/model/Address.h"
#include <vector>

namespace catapult { namespace tools {

	/// Returns server key pair used by tools.
	crypto::KeyPair LoadServerKeyPair();

	/// Returns random key pair.
	crypto::KeyPair GenerateRandomKeyPair();

	/// Generate \a count deterministic addresses.
	std::vector<Address> PrepareAddresses(size_t count);
}}
