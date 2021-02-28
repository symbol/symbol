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

#include "Bip39.h"
#include "Bip39Wordlist.h"
#include "catapult/crypto/Hashes.h"
#include <numeric>

namespace catapult { namespace extensions {

	std::string Bip39EntropyToMnemonic(const std::vector<uint8_t>& entropy) {
		if (0 != entropy.size() % 4 || entropy.size() < 16 || entropy.size() > 32)
			CATAPULT_THROW_INVALID_ARGUMENT_1("entropy size is not supported", entropy.size());

		uint16_t next = 0;
		uint8_t counter = 0;
		std::vector<std::string> words;
		auto processByte = [&next, &counter, &words](auto byte, size_t bitcount) {
			for (auto i = 0u; i < bitcount; ++i) {
				next = static_cast<uint16_t>(next << 1);
				next = static_cast<uint16_t>(next | (byte & 0x80) >> 7);
				byte = static_cast<uint8_t>(byte << 1);

				// 2^11 == 2048
				if (11 == ++counter) {
					words.push_back(Bip39_English_Wordlist[next]);
					next = 0;
					counter = 0;
				}
			}
		};

		for (auto byte : entropy)
			processByte(byte, 8);

		Hash256 checksumHash;
		crypto::Sha256(entropy, checksumHash);

		processByte(checksumHash[0], entropy.size() / 4);

		if (0 != counter)
			words.push_back(Bip39_English_Wordlist[next]);

		return std::accumulate(words.cbegin(), words.cend(), std::string(), [](const auto& lhs, const auto& rhs) {
			return lhs.empty() ? rhs : lhs + " " + rhs;
		});
	}

	Hash512 Bip39MnemonicToSeed(const std::string& mnemonic, const std::string& password) {
		auto saltedPassword = "mnemonic" + password;

		Hash512 output;
		crypto::Pbkdf2_Sha512(
				{ reinterpret_cast<const uint8_t*>(mnemonic.data()), mnemonic.size() },
				{ reinterpret_cast<const uint8_t*>(saltedPassword.data()), saltedPassword.size() },
				2048,
				output);

		return output;
	}
}}
