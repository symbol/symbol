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
#include "catapult/utils/MemoryUtils.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/test/nodeps/Random.h"
#include <tiny-aes-c/aes.hpp>
#include <cstring>

namespace catapult { namespace test {

	namespace {
		constexpr auto Aes_Pkcs7_Padding_Size = 16u;

		void Prepend(std::vector<uint8_t>& buffer, RawBuffer prefix) {
			buffer.resize(buffer.size() + prefix.Size);
			std::memmove(buffer.data() + prefix.Size, buffer.data(), buffer.size() - prefix.Size);

			std::memcpy(buffer.data(), prefix.pData, prefix.Size);
		}
	}

	void AesPkcs7PaddingScheme(std::vector<uint8_t>& buffer) {
		auto size = buffer.size();
		auto paddingSize = Aes_Pkcs7_Padding_Size;
		if (size % Aes_Pkcs7_Padding_Size)
			paddingSize = Aes_Pkcs7_Padding_Size - (size % Aes_Pkcs7_Padding_Size);

		buffer.resize(size + paddingSize);
		for (auto i = 0u; i < paddingSize; ++i)
			buffer[buffer.size() - paddingSize + i] = static_cast<uint8_t>(paddingSize);
	}

	void AesCbcEncrypt(
			const crypto::SharedKey& encryptionKey,
			const crypto::AesInitializationVector& initializationVector,
			const RawBuffer& input,
			std::vector<uint8_t>& output,
			const consumer<std::vector<uint8_t>&>& applyPaddingScheme) {
		// initializationVector || data || padding
		output.resize(input.Size);
		applyPaddingScheme(output);
		auto encryptedDataSize = output.size();

		Prepend(output, initializationVector);
		utils::memcpy_cond(output.data() + initializationVector.size(), input.pData, input.Size);

		AES_ctx ctx;
		AES_init_ctx_iv(&ctx, encryptionKey.data(), initializationVector.data());
		AES_CBC_encrypt_buffer(&ctx, output.data() + initializationVector.size(), static_cast<uint32_t>(encryptedDataSize));
	}

	std::vector<uint8_t> GenerateEphemeralAndEncrypt(const RawBuffer& clearText, const Key& recipientPublicKey) {
		auto ephemeralKeyPair = test::GenerateKeyPair();
		auto sharedKey = DeriveSharedKey(ephemeralKeyPair, recipientPublicKey);
		auto initializationVector = GenerateRandomByteArray<crypto::AesInitializationVector>();

		std::vector<uint8_t> encrypted;
		AesCbcEncrypt(sharedKey, initializationVector, clearText, encrypted);

		std::vector<uint8_t> encryptedWithKey(Key::Size + encrypted.size());
		std::memcpy(encryptedWithKey.data(), ephemeralKeyPair.publicKey().data(), Key::Size);
		std::memcpy(encryptedWithKey.data() + Key::Size, encrypted.data(), encrypted.size());

		return encryptedWithKey;
	}
}}
