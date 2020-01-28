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

#include "Hashes.h"
#include "KeccakHash.h"
#include "catapult/utils/Casting.h"
#include "catapult/utils/MemoryUtils.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#endif

extern "C" {
#include <ripemd160/ripemd160.h>
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <sha256/crypto_hash_sha256.h>

namespace catapult { namespace crypto {

	// region free functions

	void Ripemd160(const RawBuffer& dataBuffer, Hash160& hash) noexcept {
		struct ripemd160 context;
		ripemd160(&context, dataBuffer.pData, dataBuffer.Size);
		memcpy(hash.data(), context.u.u8, hash.size());
	}

	namespace {
		void Sha256(crypto_hash_sha256_state& state, const RawBuffer& dataBuffer, Hash256& hash) {
			crypto_hash_sha256_init(&state);

			if (0 < dataBuffer.Size)
				crypto_hash_sha256_update(&state, dataBuffer.pData, dataBuffer.Size);

			crypto_hash_sha256_final(&state, hash.data());
		}
	}

	void Bitcoin160(const RawBuffer& dataBuffer, Hash160& hash) noexcept {
		crypto_hash_sha256_state state;
		Hash256 firstHash;
		Sha256(state, dataBuffer, firstHash);
		Ripemd160(firstHash, hash);
	}

	void Sha256Double(const RawBuffer& dataBuffer, Hash256& hash) noexcept {
		crypto_hash_sha256_state state;
		Hash256 firstHash;
		Sha256(state, dataBuffer, firstHash);
		Sha256(state, firstHash, hash);
	}

	namespace {
		template<typename TBuilder, typename THash>
		void HashSingleBuffer(const RawBuffer& dataBuffer, THash& hash) noexcept {
			TBuilder hashBuilder;
			hashBuilder.update(dataBuffer);
			hashBuilder.final(hash);
		}
	}

	void Sha3_256(const RawBuffer& dataBuffer, Hash256& hash) noexcept {
		HashSingleBuffer<Sha3_256_Builder>(dataBuffer, hash);
	}

	void Sha3_512(const RawBuffer& dataBuffer, Hash512& hash) noexcept {
		HashSingleBuffer<Sha3_512_Builder>(dataBuffer, hash);
	}

	void Keccak_256(const RawBuffer& dataBuffer, Hash256& hash) noexcept {
		HashSingleBuffer<Keccak_256_Builder>(dataBuffer, hash);
	}

	void Keccak_512(const RawBuffer& dataBuffer, Hash512& hash) noexcept {
		HashSingleBuffer<Keccak_512_Builder>(dataBuffer, hash);
	}

	namespace {
		struct Sha256_Block_tag { static constexpr size_t Size = 64; };
		using Sha256_Block = utils::ByteArray<Sha256_Block_tag>;
	}

	void Hmac_Sha256(const RawBuffer& key, const RawBuffer& input, Hash256& output) {
		crypto_hash_sha256_state state;
		// zero-initialized by default
		Sha256_Block innerKeyPad;
		Sha256_Block outerKeyPad;

		if (key.Size > Sha256_Block::Size) {
			Hash256 tempKey;
			Sha256(state, key, tempKey);
			std::memcpy(innerKeyPad.data(), tempKey.data(), Hash256::Size);
		} else {
			utils::memcpy_cond(innerKeyPad.data(), key.pData, key.Size);
		}

		for (auto i = 0u; i < Sha256_Block::Size; ++i) {
			auto byte = innerKeyPad[i];
			innerKeyPad[i] = byte ^ 0x36;
			outerKeyPad[i] = byte ^ 0x5C;
		}

		crypto_hash_sha256_state innerState;
		Hash256 innerHash;
		crypto_hash_sha256_init(&innerState);
		crypto_hash_sha256_update(&innerState, innerKeyPad.data(), innerKeyPad.size());
		crypto_hash_sha256_update(&innerState, input.pData, input.Size);
		crypto_hash_sha256_final(&innerState, innerHash.data());

		crypto_hash_sha256_state outerState;
		crypto_hash_sha256_init(&outerState);
		crypto_hash_sha256_update(&outerState, outerKeyPad.data(), outerKeyPad.size());
		crypto_hash_sha256_update(&outerState, innerHash.data(), innerHash.size());
		crypto_hash_sha256_final(&outerState, output.data());
	}

	// endregion

	// region sha3 / keccak builders

	namespace {
		Keccak_HashInstance* CastToKeccakHashInstance(uint8_t* pHashContext) noexcept {
			return reinterpret_cast<Keccak_HashInstance*>(pHashContext);
		}

		inline void KeccakInitialize(Keccak_HashInstance* pHashContext, Hash256_tag) {
			Keccak_HashInitialize_SHA3_256(pHashContext);
		}

		inline void KeccakInitialize(Keccak_HashInstance* pHashContext, Hash512_tag) {
			Keccak_HashInitialize_SHA3_512(pHashContext);
		}

		inline void KeccakInitialize(Keccak_HashInstance* pHashContext, GenerationHash_tag) {
			Keccak_HashInitialize_SHA3_256(pHashContext);
		}

		inline void KeccakFinal(uint8_t* context, uint8_t* output, int hashSize, KeccakModeTag) noexcept {
			Keccak_HashSqueeze(CastToKeccakHashInstance(context), output, static_cast<uint32_t>(hashSize * 8));
		}

		inline void KeccakFinal(uint8_t* context, uint8_t* output, int /* ignore last argument */, Sha3ModeTag) noexcept {
			Keccak_HashFinal(CastToKeccakHashInstance(context), output);
		}
	}

	template<typename TModeTag, typename THashTag>
	KeccakBuilder<TModeTag, THashTag>::KeccakBuilder() {
		static_assert(sizeof(Keccak_HashInstance) <= sizeof(m_hashContext), "m_hashContext is too small to fit Keccak instance");
		KeccakInitialize(CastToKeccakHashInstance(m_hashContext), THashTag());
	}

	template<typename TModeTag, typename THashTag>
	void KeccakBuilder<TModeTag, THashTag>::update(const RawBuffer& dataBuffer) noexcept {
		Keccak_HashUpdate(CastToKeccakHashInstance(m_hashContext), dataBuffer.pData, dataBuffer.Size * 8);
	}

	template<typename TModeTag, typename THashTag>
	void KeccakBuilder<TModeTag, THashTag>::update(std::initializer_list<const RawBuffer> buffers) noexcept {
		for (const auto& buffer : buffers)
			update(buffer);
	}

	template<typename TModeTag, typename THashTag>
	void KeccakBuilder<TModeTag, THashTag>::final(OutputType& output) noexcept {
		KeccakFinal(m_hashContext, output.data(), THashTag::Size, TModeTag());
	}

	template class KeccakBuilder<Sha3ModeTag, Hash256_tag>;
	template class KeccakBuilder<Sha3ModeTag, Hash512_tag>;
	template class KeccakBuilder<KeccakModeTag, Hash256_tag>;
	template class KeccakBuilder<KeccakModeTag, Hash512_tag>;
	template class KeccakBuilder<Sha3ModeTag, GenerationHash_tag>;

	// endregion
}}
