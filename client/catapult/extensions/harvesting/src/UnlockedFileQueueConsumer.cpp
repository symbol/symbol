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

#include "UnlockedFileQueueConsumer.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/crypto/AesCbcDecrypt.h"
#include "catapult/io/FileQueue.h"
#include "catapult/io/RawFile.h"

namespace catapult { namespace harvesting {

	namespace {
		constexpr auto Aes_Pkcs7_Padding_Size = 16;
	}

	size_t EncryptedUnlockedEntrySize() {
		return crypto::Salt::Size
				+ crypto::AesInitializationVector::Size
				+ Key::Size
				+ Aes_Pkcs7_Padding_Size;
	}

	std::pair<crypto::PrivateKey, bool> TryDecryptUnlockedEntry(
			const RawBuffer& saltedEncrypted,
			const crypto::KeyPair& bootKeyPair,
			const Key& publicKey) {
		std::vector<uint8_t> decrypted;
		if (!crypto::TryDecryptEd25199BlockCipher(saltedEncrypted, bootKeyPair, publicKey, decrypted) || Key::Size != decrypted.size())
			return std::make_pair(crypto::PrivateKey(), false);

		return std::make_pair(crypto::PrivateKey::Generate([iter = decrypted.begin()]() mutable { return *iter++; }), true);
	}

	void UnlockedFileQueueConsumer(
			const config::CatapultDirectory& directory,
			const crypto::KeyPair& bootKeyPair,
			const consumer<const Key&, const RawBuffer&, crypto::KeyPair&&>& processEntryKeyPair) {
		io::FileQueueReader reader(directory.str());
		Key announcerPublicKey;
		auto appendMessage = [&bootKeyPair, &processEntryKeyPair, &announcerPublicKey](const std::vector<uint8_t>& buffer) {
			// filter out invalid messages
			if (Key::Size + EncryptedUnlockedEntrySize() != buffer.size())
				return;

			std::memcpy(announcerPublicKey.data(), buffer.data(), Key::Size);
			RawBuffer encrypted{ buffer.data() + Key::Size, EncryptedUnlockedEntrySize() };
			auto decryptedPair = TryDecryptUnlockedEntry(encrypted, bootKeyPair, announcerPublicKey);
			if (!decryptedPair.second)
				return;

			auto keyPair = crypto::KeyPair::FromPrivate(std::move(decryptedPair.first));
			processEntryKeyPair(announcerPublicKey, encrypted, std::move(keyPair));
		};

		while (reader.tryReadNextMessage(appendMessage))
		{}
	}
}}
