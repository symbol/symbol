#include "KeyPairTestUtils.h"

using namespace catapult::crypto;

namespace catapult { namespace test {

	KeyPair CopyKeyPair(const KeyPair& keyPair) {
		auto iter = keyPair.privateKey().begin();
		return KeyPair::FromPrivate(PrivateKey::Generate([&iter]() { return *iter++; }));
	}

	utils::KeySet ToKeySet(const std::vector<crypto::KeyPair>& keyPairs) {
		utils::KeySet publicKeys;
		for (const auto& keyPair : keyPairs)
			publicKeys.emplace(keyPair.publicKey());

		return publicKeys;
	}
}}
