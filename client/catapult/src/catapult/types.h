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

#pragma once
#include "utils/ByteArray.h"
#include "utils/ClampedBaseValue.h"
#include "utils/RawBuffer.h"
#include <array>

namespace catapult {

	// region byte arrays (ex address)

	constexpr size_t Signature_Size = 64;
	constexpr size_t Key_Size = 32;
	constexpr size_t Hash512_Size = 64;
	constexpr size_t Hash256_Size = 32;
	constexpr size_t Hash160_Size = 20;

	struct Signature_tag {};
	using Signature = utils::ByteArray<Signature_Size, Signature_tag>;

	struct Key_tag {};
	using Key = utils::ByteArray<Key_Size, Key_tag>;

	struct Hash512_tag { static constexpr auto Byte_Size = 64; };
	using Hash512 = utils::ByteArray<Hash512_Size, Hash512_tag>;

	struct Hash256_tag { static constexpr auto Byte_Size = 32; };
	using Hash256 = utils::ByteArray<Hash256_Size, Hash256_tag>;

	struct Hash160_tag {};
	using Hash160 = utils::ByteArray<Hash160_Size, Hash160_tag>;

	struct GenerationHash_tag { static constexpr auto Byte_Size = 32; };
	using GenerationHash = utils::ByteArray<Hash256_Size, GenerationHash_tag>;

	// endregion

	// region byte arrays (address)

	constexpr size_t Address_Decoded_Size = 25;
	constexpr size_t Address_Encoded_Size = 40;

	struct Address_tag {};
	using Address = utils::ByteArray<Address_Decoded_Size, Address_tag>;

	struct UnresolvedAddress_tag {};
	using UnresolvedAddress = utils::ByteArray<Address_Decoded_Size, UnresolvedAddress_tag>;

	// endregion

	// region base values

	struct Timestamp_tag {};
	using Timestamp = utils::BaseValue<uint64_t, Timestamp_tag>;

	struct Amount_tag {};
	using Amount = utils::BaseValue<uint64_t, Amount_tag>;

	struct MosaicId_tag {};
	using MosaicId = utils::BaseValue<uint64_t, MosaicId_tag>;

	struct UnresolvedMosaicId_tag {};
	using UnresolvedMosaicId = utils::BaseValue<uint64_t, UnresolvedMosaicId_tag>;

	struct Height_tag {};
	using Height = utils::BaseValue<uint64_t, Height_tag>;

	struct BlockDuration_tag {};
	using BlockDuration = utils::BaseValue<uint64_t, BlockDuration_tag>;

	struct BlockFeeMultiplier_tag {};
	using BlockFeeMultiplier = utils::BaseValue<uint32_t, BlockFeeMultiplier_tag>;

	struct Difficulty_tag {
	public:
		static constexpr uint64_t Default_Value = 100'000'000'000'000;
		static constexpr uint64_t Min_Value = Default_Value / 10;
		static constexpr uint64_t Max_Value = Default_Value * 10;
	};
	using Difficulty = utils::ClampedBaseValue<uint64_t, Difficulty_tag>;

	struct Importance_tag {};
	using Importance = utils::BaseValue<uint64_t, Importance_tag>;

	// endregion

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
