#include "KeyPairTestUtils.h"

using namespace catapult::crypto;

namespace catapult { namespace test {
	KeyPair CopyKeyPair(const KeyPair& keyPair) {
		auto iter = keyPair.privateKey().cbegin();
		return KeyPair::FromPrivate(PrivateKey::Generate([&iter]() { return *iter++; }));
	}
}}
