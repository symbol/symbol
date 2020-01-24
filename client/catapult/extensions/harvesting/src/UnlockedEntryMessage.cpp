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

#include "UnlockedEntryMessage.h"
#include "catapult/crypto/AesCbcDecrypt.h"
#include <cstring>

namespace catapult { namespace harvesting {

	namespace {
		constexpr auto Aes_Pkcs7_Padding_Size = 16;
	}

	size_t EncryptedUnlockedEntrySize() {
		// ephemeral public key | aes cbc initialization vector | encrypted harvester private key | padding
		return Key::Size
				+ crypto::AesInitializationVector::Size
				+ Key::Size
				+ Aes_Pkcs7_Padding_Size;
	}

	UnlockedEntryMessageIdentifier GetMessageIdentifier(const UnlockedEntryMessage& message) {
		UnlockedEntryMessageIdentifier messageIdentifier;
		std::memcpy(messageIdentifier.data(), message.EncryptedEntry.pData, messageIdentifier.size());
		return messageIdentifier;
	}
}}
