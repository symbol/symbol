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

#include "Hashes.h"
#include "catapult/utils/Casting.h"
#include "catapult/utils/MemoryUtils.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/ripemd.h>
#include <openssl/sha.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

namespace catapult { namespace crypto {

	// region free functions

	namespace {
		template<typename THash>
		void HashSingleBuffer(const EVP_MD* pMessageDigest, const RawBuffer& dataBuffer, THash& hash) {
			auto outputSize = static_cast<unsigned int>(hash.size());

			OpensslDigestContext context;
			context.dispatch(EVP_DigestInit_ex, pMessageDigest, nullptr);
			context.dispatch(EVP_DigestUpdate, dataBuffer.pData, dataBuffer.Size);
			context.dispatch(EVP_DigestFinal_ex, hash.data(), &outputSize);
		}
	}

	void Ripemd160(const RawBuffer& dataBuffer, Hash160& hash) {
		HashSingleBuffer(EVP_ripemd160(), dataBuffer, hash);
	}

	void Bitcoin160(const RawBuffer& dataBuffer, Hash160& hash) {
		Hash256 firstHash;
		Sha256(dataBuffer, firstHash);
		Ripemd160(firstHash, hash);
	}

	void Sha256(const RawBuffer& dataBuffer, Hash256& hash) {
		HashSingleBuffer(EVP_sha256(), dataBuffer, hash);
	}

	void Sha256Double(const RawBuffer& dataBuffer, Hash256& hash) {
		Hash256 firstHash;
		Sha256(dataBuffer, firstHash);
		Sha256(firstHash, hash);
	}

	void Sha512(const RawBuffer& dataBuffer, Hash512& hash) {
		HashSingleBuffer(EVP_sha512(), dataBuffer, hash);
	}

	void Sha3_256(const RawBuffer& dataBuffer, Hash256& hash) {
		HashSingleBuffer(EVP_sha3_256(), dataBuffer, hash);
	}

	void Hmac_Sha256(const RawBuffer& key, const RawBuffer& input, Hash256& output) {
		unsigned int outputSize = 0;
		HMAC(EVP_sha256(), key.pData, static_cast<int>(key.Size), input.pData, input.Size, output.data(), &outputSize);
	}

	void Hmac_Sha512(const RawBuffer& key, const RawBuffer& input, Hash512& output) {
		unsigned int outputSize = 0;
		HMAC(EVP_sha512(), key.pData, static_cast<int>(key.Size), input.pData, input.Size, output.data(), &outputSize);
	}

	void Pbkdf2_Sha512(const RawBuffer& password, const RawBuffer& salt, uint32_t iterationCount, Hash512& output) {
		PKCS5_PBKDF2_HMAC(
				reinterpret_cast<const char*>(password.pData),
				static_cast<int>(password.Size),
				salt.pData,
				static_cast<int>(salt.Size),
				static_cast<int>(iterationCount),
				EVP_sha512(),
				Hash512::Size,
				output.data());
	}

	// endregion

	// region hash builders

	namespace {
		const EVP_MD* GetMessageDigest(Sha2ModeTag, Hash512_tag) {
			return EVP_sha512();
		}

		const EVP_MD* GetMessageDigest(Sha3ModeTag, Hash256_tag) {
			return EVP_sha3_256();
		}

		const EVP_MD* GetMessageDigest(Sha3ModeTag, GenerationHash_tag) {
			return EVP_sha3_256();
		}
	}

	template<typename TModeTag, typename THashTag>
	HashBuilderT<TModeTag, THashTag>::HashBuilderT() {
		m_context.dispatch(EVP_DigestInit_ex, GetMessageDigest(TModeTag(), THashTag()), nullptr);
	}

	template<typename TModeTag, typename THashTag>
	void HashBuilderT<TModeTag, THashTag>::update(const RawBuffer& dataBuffer) {
		m_context.dispatch(EVP_DigestUpdate, dataBuffer.pData, dataBuffer.Size);
	}

	template<typename TModeTag, typename THashTag>
	void HashBuilderT<TModeTag, THashTag>::update(std::initializer_list<const RawBuffer> buffers) {
		for (const auto& buffer : buffers)
			update(buffer);
	}

	template<typename TModeTag, typename THashTag>
	void HashBuilderT<TModeTag, THashTag>::final(OutputType& output) {
		auto outputSize = static_cast<unsigned int>(output.size());
		m_context.dispatch(EVP_DigestFinal_ex, output.data(), &outputSize);
	}

	template class HashBuilderT<Sha2ModeTag, Hash512_tag>;
	template class HashBuilderT<Sha3ModeTag, Hash256_tag>;
	template class HashBuilderT<Sha3ModeTag, GenerationHash_tag>;

	// endregion
}}
