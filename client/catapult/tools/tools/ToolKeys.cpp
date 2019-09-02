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

#include "ToolKeys.h"
#include "Random.h"
#include "catapult/crypto/Hashes.h"
#include <random>

namespace catapult { namespace tools {

	namespace {
		// a nemesis recipient account
		constexpr auto Mijin_Test_Private_Key = "8473645728B15F007385CE2889D198D26369D2806DCDED4A9B219FD0DE23A505";

		void NextKey(Key& key) {
			Hash256 hash;
			crypto::Sha3_256(key, hash);
			std::copy(hash.cbegin(), hash.cend(), key.begin());
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
			NextKey(seedKey);
			auto address = model::PublicKeyToAddress(seedKey, model::NetworkIdentifier::Mijin_Test);

			// just to have addresses starting with 'SA'
			if (0 == (address[1] & 0xF8))
				addresses.push_back(address);
		}

		return addresses;
	}
}}
