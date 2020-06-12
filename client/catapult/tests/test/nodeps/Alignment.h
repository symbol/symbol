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
#include "catapult/crypto/Vrf.h"
#include "catapult/model/Mosaic.h"
#include "catapult/types.h"

namespace catapult { namespace test {

	/// Gets the desired alignment for type T.
	template<typename T>
	constexpr size_t GetRequiredAlignment() {
		return sizeof(T);
	}

	/// Gets the desired alignment for type Key.
	template<>
	constexpr size_t GetRequiredAlignment<Key>() {
		return sizeof(uint64_t);
	}

	/// Gets the desired alignment for type VotingKey.
	template<>
	constexpr size_t GetRequiredAlignment<VotingKey>() {
		return sizeof(uint64_t);
	}

	/// Gets the desired alignment for type Hash256.
	template<>
	constexpr size_t GetRequiredAlignment<Hash256>() {
		return sizeof(uint64_t);
	}

	/// Gets the desired alignment for type Hash512.
	template<>
	constexpr size_t GetRequiredAlignment<Hash512>() {
		return sizeof(uint64_t);
	}

	/// Gets the desired alignment for type Signature.
	template<>
	constexpr size_t GetRequiredAlignment<Signature>() {
		return sizeof(uint64_t);
	}

	/// Gets the desired alignment for type Address.
	template<>
	constexpr size_t GetRequiredAlignment<Address>() {
		return sizeof(uint8_t);
	}

	/// Gets the desired alignment for type UnresolvedAddress.
	template<>
	constexpr size_t GetRequiredAlignment<UnresolvedAddress>() {
		return sizeof(uint8_t);
	}

	/// Gets the desired alignment for type Mosaic.
	template<>
	constexpr size_t GetRequiredAlignment<model::Mosaic>() {
		return sizeof(uint64_t);
	}

	/// Gets the desired alignment for type UnresolvedMosaic.
	template<>
	constexpr size_t GetRequiredAlignment<model::UnresolvedMosaic>() {
		return sizeof(uint64_t);
	}

	/// Gets the desired alignment for type GenerationHashSeed.
	template<>
	constexpr size_t GetRequiredAlignment<GenerationHashSeed>() {
		return sizeof(uint64_t);
	}

	/// Gets the desired alignment for type ProofGamma.
	template<>
	constexpr size_t GetRequiredAlignment<crypto::ProofGamma>() {
		return sizeof(uint64_t);
	}

	/// Gets the desired alignment for type ProofVerificationHash.
	template<>
	constexpr size_t GetRequiredAlignment<crypto::ProofVerificationHash>() {
		return sizeof(uint64_t);
	}

	/// Gets the desired alignment for type ProofScalar.
	template<>
	constexpr size_t GetRequiredAlignment<crypto::ProofScalar>() {
		return sizeof(uint64_t);
	}
}}

/// Asserts that \a FIELD has proper alignment within \a STRUCT.
#define EXPECT_ALIGNED_(STRUCT, FIELD) EXPECT_EQ(0u, offsetof(STRUCT, FIELD) % test::GetRequiredAlignment<decltype(STRUCT::FIELD)>())

#ifndef _MSC_VER
#define EXPECT_ALIGNED_WITH_PRAGMAS(STRUCT, FIELD) \
	do { \
		_Pragma("GCC diagnostic push") \
		_Pragma("GCC diagnostic ignored \"-Winvalid-offsetof\"") /* allow offsetof on non-standard layout type */ \
		EXPECT_ALIGNED_(STRUCT, FIELD); \
		_Pragma("GCC diagnostic pop") \
	} while (false)

// extra indirection is needed for GCC Release builds
#define EXPECT_ALIGNED EXPECT_ALIGNED_WITH_PRAGMAS
#else
#define EXPECT_ALIGNED EXPECT_ALIGNED_
#endif
