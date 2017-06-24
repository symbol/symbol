#pragma once
#include "catapult/types.h"

namespace catapult { namespace crypto {

	/// Calculates the ripemd160 hash of \a dataBuffer into \a hash.
	void Ripemd160(const RawBuffer& dataBuffer, Hash160& hash) noexcept;

	/// Calculates the 256-bit SHA3 hash of \a dataBuffer into \a hash.
	void Sha3_256(const RawBuffer& dataBuffer, Hash256& hash) noexcept;

	/// Calculates the 512-bit SHA3 hash of \a dataBuffer into \a hash.
	void Sha3_512(const RawBuffer& dataBuffer, Hash512& hash) noexcept;

	/// Wraps 256-bit sha3 into an object.
	class alignas(32) Sha3_256_Builder {
	public:
		using OutputType = Hash256;

		/// Creates instance of sha3.
		Sha3_256_Builder();

		/// Updates state of hash with data inside \a dataBuffer.
		void update(const RawBuffer& dataBuffer) noexcept;

		/// Updates the state of hash with concatenated \a buffersList.
		void update(std::initializer_list<const RawBuffer> buffersList) noexcept;

		/// Finalize sha3 calculation. Returns result in \a output.
		void final(OutputType& output) noexcept;

	private:
		// Size below is related to amount of data Keccak needs for
		// its internal state.
		uint8_t m_hashContext[256];
	};

	/// Wraps 512-bit sha3 into an object.
	class alignas(32) Sha3_512_Builder {
	public:
		using OutputType = Hash512;

		/// Creates instance of sha3.
		Sha3_512_Builder();

		/// Updates the state of hash with data inside \a dataBuffer.
		void update(const RawBuffer& dataBuffer) noexcept;

		/// Updates the state of hash with concatenated \a buffersList.
		void update(std::initializer_list<const RawBuffer> buffersList) noexcept;

		/// Finalize sha3 calculation. Returns result in \a output.
		void final(OutputType& output) noexcept;

	private:
		// Size below is related to amount of data Keccak needs for
		// its internal state.
		uint8_t m_hashContext[256];
	};
}}
