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

#include "HarvestRequest.h"
#include "catapult/crypto/AesDecrypt.h"
#include <cstring>

namespace catapult { namespace harvesting {

	size_t HarvestRequest::DecryptedPayloadSize() {
		// encrypted harvester signing private key | encrypted harvester vrf private key
		return 2 * Key::Size;
	}

	size_t HarvestRequest::EncryptedPayloadSize() {
		return Key::Size //                       ephemeral public key
				+ crypto::AesGcm256::Tag::Size // aes gcm tag
				+ crypto::AesGcm256::IV::Size //  aes gcm initialization vector
				+ DecryptedPayloadSize(); //      decrypted payload
	}

	HarvestRequestIdentifier GetRequestIdentifier(const HarvestRequest& request) {
		HarvestRequestIdentifier requestIdentifier;
		std::memcpy(requestIdentifier.data(), request.EncryptedPayload.pData, requestIdentifier.size());
		return requestIdentifier;
	}
}}
