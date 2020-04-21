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
		constexpr auto Decrypted_Data_Size = 2 * Key::Size;

		UnlockedEntryMessage DeserializeUnlockedEntryMessage(const std::vector<uint8_t>& buffer) {
			UnlockedEntryMessage message;
			// note: value of direction comes from TransferMessageObserver, so it is trusted
			message.Direction = static_cast<UnlockedEntryDirection>(buffer[0]);
			message.EncryptedEntry = RawBuffer{ &buffer[1], EncryptedUnlockedEntrySize() };
			return message;
		}
	}

	std::pair<BlockGeneratorAccountDescriptor, bool> TryDecryptBlockGeneratorAccountDescriptor(
			const RawBuffer& encryptedWithKey,
			const crypto::KeyPair& encryptionKeyPair) {
		std::vector<uint8_t> decrypted;
		auto isDecryptSuccessful = crypto::TryDecryptEd25199BlockCipher(encryptedWithKey, encryptionKeyPair, decrypted);
		if (!isDecryptSuccessful || Decrypted_Data_Size != decrypted.size())
			return std::make_pair(BlockGeneratorAccountDescriptor(), false);

		auto iter = decrypted.begin();
		auto extractKeyPair = [&iter]() {
			return crypto::KeyPair::FromPrivate(crypto::PrivateKey::Generate([&iter]() mutable { return *iter++; }));
		};

		auto signingKeyPair = extractKeyPair();
		auto vrfKeyPair = extractKeyPair();
		return std::make_pair(BlockGeneratorAccountDescriptor(std::move(signingKeyPair), std::move(vrfKeyPair)), true);
	}

	void UnlockedFileQueueConsumer(
			const config::CatapultDirectory& directory,
			const crypto::KeyPair& encryptionKeyPair,
			const consumer<const UnlockedEntryMessage&, BlockGeneratorAccountDescriptor&&>& processDescriptor) {
		io::FileQueueReader reader(directory.str());
		auto appendMessage = [&encryptionKeyPair, &processDescriptor](const auto& buffer) {
			// filter out invalid messages
			if (1 + EncryptedUnlockedEntrySize() != buffer.size()) {
				CATAPULT_LOG(warning) << "rejecting buffer with wrong size: " << buffer.size();
				return;
			}

			auto unlockedEntryMessage = DeserializeUnlockedEntryMessage(buffer);
			auto decryptedPair = TryDecryptBlockGeneratorAccountDescriptor(unlockedEntryMessage.EncryptedEntry, encryptionKeyPair);
			if (!decryptedPair.second) {
				CATAPULT_LOG(warning) << "rejecting buffer that could not be decrypted";
				return;
			}

			processDescriptor(unlockedEntryMessage, std::move(decryptedPair.first));
		};

		while (reader.tryReadNextMessage(appendMessage))
		{}
	}
}}
