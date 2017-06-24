#include "Hashes.h"
#include "KeccakHash.h"
#include "catapult/utils/Casting.h"

extern "C" {
#include <ripemd160/rmd160.h>
}

namespace catapult { namespace crypto {

	void Ripemd160(const RawBuffer& dataBuffer, Hash160& hash) noexcept {
		ripemd(hash.data(), dataBuffer.pData, static_cast<dword>(dataBuffer.Size));
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

#ifdef NIS1_COMPATIBLE_SIGNATURES
		inline void KeccakFinal(uint8_t* context, uint8_t* output, int hashSize) noexcept {
			Keccak_HashSqueeze(CastToKeccakHashInstance(context), output, hashSize * 8);
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
