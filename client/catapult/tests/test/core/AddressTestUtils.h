#pragma once
#include "catapult/crypto/KeyPair.h"
#include "catapult/crypto/PrivateKey.h"
#include "catapult/model/Address.h"
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
	inline Address GenerateRandomAddress(const model::NetworkIdentifier networkIdentifier) {
		auto publicKey = GenerateRandomData<Key_Size>();
		return model::PublicKeyToAddress(publicKey, networkIdentifier);
	}

	/// Generates a random address.
	inline Address GenerateRandomAddress() {
		return GenerateRandomAddress(model::NetworkIdentifier::Mijin_Test);
	}
}}
