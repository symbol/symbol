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

#include "UnlockedFileQueueConsumer.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/crypto/AesDecrypt.h"
#include "catapult/io/FileQueue.h"
#include "catapult/io/RawFile.h"
#include "catapult/utils/Logging.h"

namespace catapult { namespace harvesting {

	namespace {
		size_t ExpectedSerializedHarvestRequestSize() {
			return 1 + sizeof(Height) + Key::Size + HarvestRequest::EncryptedPayloadSize();
		}
	}

	std::pair<BlockGeneratorAccountDescriptor, bool> TryDecryptBlockGeneratorAccountDescriptor(
			const RawBuffer& publicKeyPrefixedEncryptedPayload,
			const crypto::KeyPair& encryptionKeyPair) {
		std::vector<uint8_t> decrypted;
		auto isDecryptSuccessful = crypto::TryDecryptEd25199BlockCipher(publicKeyPrefixedEncryptedPayload, encryptionKeyPair, decrypted);
		if (!isDecryptSuccessful || HarvestRequest::DecryptedPayloadSize() != decrypted.size())
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
			Height maxHeight,
			const crypto::KeyPair& encryptionKeyPair,
			const consumer<const HarvestRequest&, BlockGeneratorAccountDescriptor&&>& processDescriptor) {
		io::FileQueueReader reader(directory.str());
		auto appendMessage = [maxHeight, &encryptionKeyPair, &processDescriptor](const auto& buffer) {
			// filter out invalid requests
			if (ExpectedSerializedHarvestRequestSize() != buffer.size()) {
				CATAPULT_LOG(warning) << "rejecting buffer with wrong size: " << buffer.size();
				return true;
			}

			auto harvestRequest = DeserializeHarvestRequest(buffer);
			if (maxHeight < harvestRequest.Height) {
				CATAPULT_LOG(debug) << "skipping request with height " << harvestRequest.Height << " given max height " << maxHeight;
				return false; // not fully processed
			}

			auto decryptedPair = TryDecryptBlockGeneratorAccountDescriptor(harvestRequest.EncryptedPayload, encryptionKeyPair);
			if (!decryptedPair.second) {
				CATAPULT_LOG(warning) << "rejecting request with encrypted payload that could not be decrypted";
				return true;
			}

			processDescriptor(harvestRequest, std::move(decryptedPair.first));
			return true;
		};

		while (reader.tryReadNextMessageConditional(appendMessage))
		{}
	}
}}
