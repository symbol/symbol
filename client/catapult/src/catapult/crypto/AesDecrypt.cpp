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

#include "AesDecrypt.h"
#include "OpensslContexts.h"
#include "SecureZero.h"
#include <cstring>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <openssl/evp.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

namespace catapult { namespace crypto {

	bool AesGcm256::TryDecrypt(const SharedKey& key, const RawBuffer& input, std::vector<uint8_t>& output) {
		if (input.Size < Tag::Size + IV::Size)
			return false;

		Tag tag;
		IV iv;
		std::memcpy(tag.data(), input.pData, tag.size());
		std::memcpy(iv.data(), input.pData + Tag::Size, iv.size());

		output.resize(input.Size - Tag::Size - IV::Size);
		auto outputSize = static_cast<int>(output.size());

		OpensslCipherContext cipherContext;
		cipherContext.dispatch(EVP_DecryptInit_ex, EVP_aes_256_gcm(), nullptr, key.data(), iv.data());
		cipherContext.dispatch(EVP_CIPHER_CTX_ctrl, EVP_CTRL_GCM_SET_TAG, 16, tag.data());
		cipherContext.dispatch(EVP_DecryptUpdate, output.data(), &outputSize, input.pData + Tag::Size + IV::Size, outputSize);
		return cipherContext.tryDispatch(EVP_DecryptFinal_ex, output.data() + outputSize, &outputSize);
	}

	namespace {
		RawBuffer SubView(const RawBuffer& rawBuffer, size_t offset) {
			return { rawBuffer.pData + offset, rawBuffer.Size - offset };
		}
	}

	bool TryDecryptEd25199BlockCipher(
			const RawBuffer& publicKeyPrefixedEncryptedPayload,
			const KeyPair& keyPair,
			std::vector<uint8_t>& decrypted) {
		if (Key::Size > publicKeyPrefixedEncryptedPayload.Size)
			CATAPULT_THROW_INVALID_ARGUMENT("encrypted data does not contain key");

		Key ephemeralPublicKey;
		std::memcpy(ephemeralPublicKey.data(), publicKeyPrefixedEncryptedPayload.pData, Key::Size);

		auto sharedKey = DeriveSharedKey(keyPair, ephemeralPublicKey);
		auto encryptedData = SubView(publicKeyPrefixedEncryptedPayload, Key::Size);
		bool success = AesGcm256::TryDecrypt(sharedKey, encryptedData, decrypted);
		SecureZero(sharedKey);
		return success;
	}
}}
