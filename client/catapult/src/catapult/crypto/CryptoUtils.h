#pragma once
#include "catapult/types.h"

namespace catapult { namespace crypto { class PrivateKey; } }

namespace catapult { namespace crypto {

	/// Calculates \a hash of a \a privateKey.
	void HashPrivateKey(const PrivateKey& privateKey, Hash512& hash);
}}
