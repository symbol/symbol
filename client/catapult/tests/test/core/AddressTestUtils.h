/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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
#include "sdk/src/extensions/ConversionExtensions.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/crypto/PrivateKey.h"
#include "catapult/model/Address.h"
#include "catapult/model/ContainerTypes.h"
#include "catapult/model/NetworkInfo.h"
#include "tests/test/nodeps/Random.h"

namespace catapult { namespace test {

	/// Generates a random private key.
	inline crypto::PrivateKey GenerateRandomPrivateKey() {
		return crypto::PrivateKey::Generate(test::RandomByte);
	}

	/// Generates a random key pair.
	inline crypto::KeyPair GenerateKeyPair() {
		return crypto::KeyPair::FromPrivate(GenerateRandomPrivateKey());
	}

	/// Generates a random address for a given network id (\a networkIdentifier).
	inline Address GenerateRandomAddress(model::NetworkIdentifier networkIdentifier) {
		auto publicKey = GenerateRandomData<Key_Size>();
		return model::PublicKeyToAddress(publicKey, networkIdentifier);
	}

	/// Generates a random address.
	inline Address GenerateRandomAddress() {
		return GenerateRandomAddress(model::NetworkIdentifier::Mijin_Test);
	}

	/// Generates a random (unresolved) address.
	inline UnresolvedAddress GenerateRandomUnresolvedAddress() {
		return extensions::CopyToUnresolvedAddress(GenerateRandomAddress());
	}

	/// Generates a random set of \a count addresses.
	inline model::AddressSet GenerateRandomAddressSet(size_t count) {
		model::AddressSet addresses;
		for (auto i = 0u; i < count; ++i)
			addresses.emplace(test::GenerateRandomData<Address_Decoded_Size>());

		return addresses;
	}
}}
