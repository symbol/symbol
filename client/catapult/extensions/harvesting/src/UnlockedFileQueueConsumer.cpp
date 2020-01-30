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
#include "catapult/utils/Logging.h"

namespace catapult { namespace harvesting {

	namespace {
		UnlockedEntryMessage DeserializeUnlockedEntryMessage(const std::vector<uint8_t>& buffer) {
			UnlockedEntryMessage message;
			// note: value of direction comes from TransferMessageObserver, so it is trusted
			message.Direction = static_cast<UnlockedEntryDirection>(buffer[0]);
			message.EncryptedEntry = RawBuffer{ &buffer[1], EncryptedUnlockedEntrySize() };
			return message;
		}
	}

	std::pair<crypto::PrivateKey, bool> TryDecryptUnlockedEntry(const RawBuffer& encryptedWithKey, const crypto::KeyPair& bootKeyPair) {
		std::vector<uint8_t> decrypted;
		if (!crypto::TryDecryptEd25199BlockCipher(encryptedWithKey, bootKeyPair, decrypted) || Key::Size != decrypted.size())
			return std::make_pair(crypto::PrivateKey(), false);

		return std::make_pair(crypto::PrivateKey::Generate([iter = decrypted.begin()]() mutable { return *iter++; }), true);
	}

	void UnlockedFileQueueConsumer(
			const config::CatapultDirectory& directory,
			const crypto::KeyPair& bootKeyPair,
			const consumer<const UnlockedEntryMessage&, crypto::KeyPair&&>& processEntryKeyPair) {
		io::FileQueueReader reader(directory.str());
		auto appendMessage = [&bootKeyPair, &processEntryKeyPair](const std::vector<uint8_t>& buffer) {
			// filter out invalid messages
			if (1 + EncryptedUnlockedEntrySize() != buffer.size()) {
				CATAPULT_LOG(warning) << "rejecting buffer with wrong size: " << buffer.size();
				return;
			}

			auto unlockedEntryMessage = DeserializeUnlockedEntryMessage(buffer);
			auto decryptedPair = TryDecryptUnlockedEntry(unlockedEntryMessage.EncryptedEntry, bootKeyPair);
			if (!decryptedPair.second) {
				CATAPULT_LOG(warning) << "rejecting buffer that could not be decrypted";
				return;
			}

			auto keyPair = crypto::KeyPair::FromPrivate(std::move(decryptedPair.first));
			processEntryKeyPair(unlockedEntryMessage, std::move(keyPair));
		};

		while (reader.tryReadNextMessage(appendMessage))
		{}
	}
}}
