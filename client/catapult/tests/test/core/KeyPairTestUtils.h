#pragma once
#include "catapult/crypto/KeyPair.h"
#include "catapult/utils/ArraySet.h"
#include <vector>

namespace catapult { namespace test {

	/// Copies a given \a keyPair.
	crypto::KeyPair CopyKeyPair(const crypto::KeyPair& keyPair);

	/// Extracts the public keys of \a keyPairs into a key set.
	utils::KeySet ToKeySet(const std::vector<crypto::KeyPair>& keyPairs);
}}
