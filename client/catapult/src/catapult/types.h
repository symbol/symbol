#pragma once
#include "catapult/utils/ClampedBaseValue.h"
#include "catapult/utils/RawBuffer.h"
#include <array>

namespace catapult {
	constexpr size_t Signature_Size = 64;
	constexpr size_t Hash512_Size = 64;
	constexpr size_t Hash256_Size = 32;
	constexpr size_t Hash160_Size = 20;
	constexpr size_t Key_Size = 32;
	constexpr size_t Address_Decoded_Size = 25;
	constexpr size_t Address_Encoded_Size = 40;

	using Signature = std::array<uint8_t, Signature_Size>;
	using Key = std::array<uint8_t, Key_Size>;
	using Hash512 = std::array<uint8_t, Hash512_Size>;
	using Hash256 = std::array<uint8_t, Hash256_Size>;
	using Hash160 = std::array<uint8_t, Hash160_Size>;
	using Address = std::array<uint8_t, Address_Decoded_Size>;

	struct Timestamp_tag {};
	using Timestamp = utils::BaseValue<uint64_t, Timestamp_tag>;

	struct Amount_tag {};
	using Amount = utils::BaseValue<uint64_t, Amount_tag>;

	struct MosaicId_tag {};
	using MosaicId = utils::BaseValue<uint64_t, MosaicId_tag>;

	struct Height_tag {};
	using Height = utils::BaseValue<uint64_t, Height_tag>;

	struct Difficulty_tag {
	public:
		static constexpr uint64_t Default_Value = 100'000'000'000'000;
		static constexpr uint64_t Min_Value = Default_Value / 10;
		static constexpr uint64_t Max_Value = Default_Value * 10;
	};
	using Difficulty = utils::ClampedBaseValue<uint64_t, Difficulty_tag>;

	struct Importance_tag {};
	using Importance = utils::BaseValue<uint64_t, Importance_tag>;

	using utils::RawBuffer;
	using utils::MutableRawBuffer;
	using utils::RawString;
	using utils::MutableRawString;

	/// Returns the size of the specified array.
	template<typename T, size_t N>
	constexpr size_t CountOf(T const (&)[N]) noexcept {
		return N;
	}
}
