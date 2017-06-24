#include "ToolKeys.h"
#include "catapult/crypto/Hashes.h"
#include <boost/exception/diagnostic_information.hpp>
#include <random>

namespace catapult { namespace tools {
	namespace {
		// a nemesis recipient account
		constexpr auto Mijin_Test_Private_Key = "8473645728B15F007385CE2889D198D26369D2806DCDED4A9B219FD0DE23A505";

		uint8_t RandomByte() {
			std::random_device rd;
			std::mt19937_64 gen;
			auto seed = (static_cast<uint64_t>(rd()) << 32) | rd();
			gen.seed(seed);
			return static_cast<uint8_t>(gen());
		}
	}

	crypto::KeyPair LoadServerKeyPair() {
		return crypto::KeyPair::FromString(Mijin_Test_Private_Key);
	}

	crypto::KeyPair GenerateRandomKeyPair() {
		return crypto::KeyPair::FromPrivate(crypto::PrivateKey::Generate(RandomByte));
	}

	std::vector<Address> PrepareAddresses(size_t count) {
		std::vector<Address> addresses;
		auto seedKey = Key();

		addresses.reserve(count);
		while (count != addresses.size()) {
			crypto::Sha3_256(seedKey, seedKey);
			auto address = model::PublicKeyToAddress(seedKey, model::NetworkIdentifier::Mijin_Test);

			// just to have addresses starting with 'SA'
			if (0 == (address[1] & 0xf8))
				addresses.push_back(address);
		}

		return addresses;
	}
}}
