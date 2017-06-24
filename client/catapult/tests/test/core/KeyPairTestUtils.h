#pragma once
#include "catapult/crypto/KeyPair.h"

namespace catapult { namespace test {
	/// Copies a given \a keyPair.
	crypto::KeyPair CopyKeyPair(const crypto::KeyPair& keyPair);
}}
