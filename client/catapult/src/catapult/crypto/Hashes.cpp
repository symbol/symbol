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

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#endif

extern "C" {
#include <ripemd160/ripemd160.h>
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

namespace catapult { namespace crypto {

	void Ripemd160(const RawBuffer& dataBuffer, Hash160& hash) noexcept {
		struct ripemd160 context;
		ripemd160(&context, dataBuffer.pData, dataBuffer.Size);
		memcpy(hash.data(), context.u.u8, Hash160_Size);
	}

	void Sha3_256(const RawBuffer& dataBuffer, Hash256& hash) noexcept {
		Sha3_256_Builder sha3;
		sha3.update(dataBuffer);
		sha3.final(hash);
	}

	void Sha3_512(const RawBuffer& dataBuffer, Hash512& hash) noexcept {
		Sha3_512_Builder sha3;
		sha3.update(dataBuffer);
		sha3.final(hash);
	}

	namespace {
		auto CastToKeccakHashInstance(uint8_t* pHashContext) noexcept {
			return reinterpret_cast<Keccak_HashInstance*>(pHashContext);
		}

#ifdef SIGNATURE_SCHEME_NIS1
		inline void KeccakFinal(uint8_t* context, uint8_t* output, int hashSize) noexcept {
			Keccak_HashSqueeze(CastToKeccakHashInstance(context), output, static_cast<uint32_t>(hashSize * 8));
		}
#else
		inline void KeccakFinal(uint8_t* context, uint8_t* output, int /* ignore last argument */) noexcept {
			Keccak_HashFinal(CastToKeccakHashInstance(context), output);
		}
#endif
	}
	Sha3_256_Builder::Sha3_256_Builder() {
		static_assert(sizeof(Keccak_HashInstance) <= sizeof(m_hashContext), "Provided m_hashContext is too small to fit Keccak instance.");
		Keccak_HashInitialize_SHA3_256(CastToKeccakHashInstance(m_hashContext));
	}

	void Sha3_256_Builder::update(const RawBuffer& dataBuffer) noexcept {
		Keccak_HashUpdate(CastToKeccakHashInstance(m_hashContext), dataBuffer.pData, dataBuffer.Size * 8);
	}

	void Sha3_256_Builder::update(std::initializer_list<const RawBuffer> buffersList) noexcept {
		for (const auto& buffer : buffersList)
			update(buffer);
	}

	void Sha3_256_Builder::final(OutputType& output) noexcept {
		KeccakFinal(m_hashContext, output.data(), std::tuple_size<OutputType>::value);
	}

	Sha3_512_Builder::Sha3_512_Builder() {
		static_assert(sizeof(Keccak_HashInstance) <= sizeof(m_hashContext), "Provided m_hashContext is too small to fit Keccak instance.");
		Keccak_HashInitialize_SHA3_512(CastToKeccakHashInstance(m_hashContext));
	}

	void Sha3_512_Builder::update(const RawBuffer& dataBuffer) noexcept {
		Keccak_HashUpdate(CastToKeccakHashInstance(m_hashContext), dataBuffer.pData, dataBuffer.Size * 8);
	}

	void Sha3_512_Builder::update(std::initializer_list<const RawBuffer> buffersList) noexcept {
		for (const auto& buffer : buffersList)
			update(buffer);
	}

	void Sha3_512_Builder::final(OutputType& output) noexcept {
		KeccakFinal(m_hashContext, output.data(), std::tuple_size<OutputType>::value);
	}
}}
