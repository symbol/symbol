#pragma once
#include "catapult/types.h"

namespace catapult { namespace cache { class CatapultCacheView; } }

namespace catapult { namespace test {

	/// Converts a raw private key string (\a privateKeyString) to an address.
	Address RawPrivateKeyToAddress(const char* privateKeyString);

	/// Converts a raw public key string (\a publicKeyString) to an address.
	Address RawPublicKeyToAddress(const char* publicKeyString);

	/// Asserts that \a view contains expected nemesis account state.
	void AssertNemesisAccountState(const cache::CatapultCacheView& view);

	/// Asserts that \a view contains expected nemesis mosaic state.
	void AssertNemesisMosaicState(const cache::CatapultCacheView& view);

	/// Asserts that \a view contains expected nemesis namespace state.
	void AssertNemesisNamespaceState(const cache::CatapultCacheView& view);
}}
