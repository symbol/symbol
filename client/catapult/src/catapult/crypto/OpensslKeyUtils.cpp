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

#include "OpensslKeyUtils.h"
#include "catapult/exceptions.h"
#include <fstream>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <openssl/pem.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

namespace catapult { namespace crypto {

	namespace {
		template<typename TTransformer>
		auto TransformPemFileContents(const std::string& filename, TTransformer transformer) {
			std::ifstream fin(filename, std::ios_base::in);
			std::string pemStr((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());

			auto pMemBio = std::shared_ptr<BIO>(BIO_new_mem_buf(pemStr.data(), static_cast<int>(pemStr.size())), BIO_free);
			if (!pMemBio)
				throw std::bad_alloc();

			auto* pKey = PEM_read_bio_PrivateKey(pMemBio.get(), nullptr, nullptr, nullptr);
			if (!pKey)
				CATAPULT_THROW_INVALID_ARGUMENT_1("could not load private key from file", filename);

			auto transformerResult = transformer(*pKey);
			if (!transformerResult.second)
				CATAPULT_THROW_INVALID_ARGUMENT_1("could not extract raw key from file", filename);

			return std::move(transformerResult.first);
		}
	}

	Key ReadPublicKeyFromPrivateKeyPemFile(const std::string& filename) {
		return TransformPemFileContents(filename, [](const auto& key) {
			Key publicKey;
			auto publicKeyBufferSize = Key::Size;
			auto result = EVP_PKEY_get_raw_public_key(&key, publicKey.data(), &publicKeyBufferSize);
			return std::make_pair(publicKey, result);
		});
	}

	crypto::KeyPair ReadKeyPairFromPrivateKeyPemFile(const std::string& filename) {
		return TransformPemFileContents(filename, [](const auto& key) {
			std::array<uint8_t, Key::Size> privateKeyBuffer;
			auto privateKeyBufferSize = Key::Size;
			if (!EVP_PKEY_get_raw_private_key(&key, privateKeyBuffer.data(), &privateKeyBufferSize))
				return std::make_pair(KeyPair::FromPrivate(PrivateKey()), false);

			auto i = 0u;
			auto privateKey = PrivateKey::Generate([&privateKeyBuffer, &i]() {
				return privateKeyBuffer[i++];
			});
			return std::make_pair(KeyPair::FromPrivate(std::move(privateKey)), true);
		});
	}
}}
