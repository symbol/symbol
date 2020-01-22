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

#include "AesCbcDecrypt.h"
#include "catapult/exceptions.h"
#include <tiny-aes-c/aes.hpp>
#include <cstring>

namespace catapult { namespace crypto {

	namespace {
		bool DropPadding(std::vector<uint8_t>& result) {
			if (result.size() < AES_BLOCKLEN)
				return false;

			auto padByte = result.back();
			if (0 == padByte || padByte > AES_BLOCKLEN)
				return false;

			// check padding
			if (!std::all_of(result.rbegin(), result.rbegin() + padByte, [padByte](auto currentByte) { return currentByte == padByte; }))
				return false;

			result.resize(result.size() - padByte);
			return true;
		}
	}

	bool TryAesCbcDecrypt(const SharedKey& key, const RawBuffer& input, std::vector<uint8_t>& output) {
		AesInitializationVector initializationVector;
		if (input.Size < initializationVector.size())
			return false;

		output.resize(input.Size - initializationVector.size());
		if (0 != output.size() % AES_BLOCKLEN)
			return false;

		std::memcpy(initializationVector.data(), input.pData, initializationVector.size());
		std::memcpy(output.data(), input.pData + initializationVector.size(), output.size());

		AES_ctx ctx;
		AES_init_ctx_iv(&ctx, key.data(), initializationVector.data());
		AES_CBC_decrypt_buffer(&ctx, output.data(), static_cast<uint32_t>(output.size()));

		// drop PKCS#7 padding
		if (!DropPadding(output))
			return false;

		return true;
	}

	namespace {
		RawBuffer SubView(const RawBuffer& rawBuffer, size_t offset) {
			return { rawBuffer.pData + offset, rawBuffer.Size - offset };
		}
	}

	bool TryDecryptEd25199BlockCipher(const RawBuffer& encryptedWithKey, const KeyPair& keyPair, std::vector<uint8_t>& decrypted) {
		if (Key::Size > encryptedWithKey.Size)
			CATAPULT_THROW_INVALID_ARGUMENT("encrypted data does not contain key");

		Key ephemeralPublicKey;
		std::memcpy(ephemeralPublicKey.data(), encryptedWithKey.pData, Key::Size);

		auto sharedKey = DeriveSharedKey(keyPair, ephemeralPublicKey);
		auto encryptedData = SubView(encryptedWithKey, Key::Size);
		return TryAesCbcDecrypt(sharedKey, encryptedData, decrypted);
	}
}}
