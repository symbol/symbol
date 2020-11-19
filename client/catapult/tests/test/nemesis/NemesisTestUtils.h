/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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
#include "catapult/types.h"

namespace catapult { namespace cache { class CatapultCacheView; } }

namespace catapult { namespace test {

	/// Converts a raw private key string (\a privateKeyString) to a public key.
	Key RawPrivateKeyToPublicKey(const char* privateKeyString);

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
