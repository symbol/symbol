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

#include "EncryptionTestUtils.h"
#include "catapult/crypto/OpensslContexts.h"
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/test/nodeps/Random.h"
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

namespace catapult { namespace test {

	namespace {
		void Prepend(std::vector<uint8_t>& buffer, RawBuffer prefix) {
			buffer.resize(buffer.size() + prefix.Size);
			std::memmove(buffer.data() + prefix.Size, buffer.data(), buffer.size() - prefix.Size);

			std::memcpy(buffer.data(), prefix.pData, prefix.Size);
		}
	}

	void AesGcmEncrypt(
			const crypto::SharedKey& encryptionKey,
			const crypto::AesGcm256::IV& iv,
			const RawBuffer& input,
			std::vector<uint8_t>& output) {
		// encrypt input into output
		output.resize(input.Size);
		auto outputSize = static_cast<int>(output.size());
		utils::memcpy_cond(output.data(), input.pData, input.Size);

		crypto::OpensslCipherContext cipherContext;
		cipherContext.dispatch(EVP_EncryptInit_ex, EVP_aes_256_gcm(), nullptr, encryptionKey.data(), iv.data());

		if (0 != outputSize)
			cipherContext.dispatch(EVP_EncryptUpdate, output.data(), &outputSize, output.data(), outputSize);

		cipherContext.dispatch(EVP_EncryptFinal_ex, output.data() + outputSize, &outputSize);

		// get tag
		crypto::AesGcm256::Tag tag;
		cipherContext.dispatch(EVP_CIPHER_CTX_ctrl, EVP_CTRL_GCM_GET_TAG, 16, tag.data());

		// tag || iv || data
		Prepend(output, iv);
		Prepend(output, tag);
	}

	std::vector<uint8_t> GenerateEphemeralAndEncrypt(const RawBuffer& clearText, const Key& recipientPublicKey) {
		auto ephemeralKeyPair = test::GenerateKeyPair();
		auto sharedKey = DeriveSharedKey(ephemeralKeyPair, recipientPublicKey);
		auto iv = GenerateRandomByteArray<crypto::AesGcm256::IV>();

		std::vector<uint8_t> encrypted;
		AesGcmEncrypt(sharedKey, iv, clearText, encrypted);

		std::vector<uint8_t> publicKeyPrefixedEncryptedPayload(Key::Size + encrypted.size());
		std::memcpy(publicKeyPrefixedEncryptedPayload.data(), ephemeralKeyPair.publicKey().data(), Key::Size);
		std::memcpy(publicKeyPrefixedEncryptedPayload.data() + Key::Size, encrypted.data(), encrypted.size());
		return publicKeyPrefixedEncryptedPayload;
	}
}}
