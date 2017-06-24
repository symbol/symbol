#pragma once
#include "catapult/types.h"

namespace catapult { namespace cache { class CatapultCacheView; } }

namespace catapult { namespace test {

	/// Converts a raw private key string (\a pPrivateKeyString) to an address.
	Address RawPrivateKeyToAddress(const char* pPrivateKeyString);

	/// Converts a raw public key string (\a pPublicKeyString) to an address.
	Address RawPublicKeyToAddress(const char* pPublicKeyString);

	/// Asserts that \a view contains expected nemesis account state.
	void AssertNemesisAccountState(const cache::CatapultCacheView& view);

	/// Asserts that \a view contains expected nemesis mosaic state.
	void AssertNemesisMosaicState(const cache::CatapultCacheView& view);

	/// Asserts that \a view contains expected nemesis namespace state.
	void AssertNemesisNamespaceState(const cache::CatapultCacheView& view);
}}
