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

#include "MetadataTestUtils.h"
#include "src/state/MetadataEntry.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/utils/Casting.h"
#include "tests/test/core/ResolverTestUtils.h"
#include "tests/TestHarness.h"
#include <numeric>

namespace catapult { namespace test {

	// region PartialMetadataKey

	model::PartialMetadataKey GenerateRandomPartialMetadataKey() {
		return GenerateRandomPartialMetadataKey(Random());
	}

	model::PartialMetadataKey GenerateRandomPartialMetadataKey(uint64_t scopedMetadataKey) {
		return { GenerateRandomByteArray<Address>(), GenerateRandomByteArray<Address>(), scopedMetadataKey };
	}

	// endregion

	// region MetadataKey

	Hash256 CreateMetadataUniqueKeyFromSeed(uint8_t seed) {
		std::array<uint8_t, 2 * Key::Size + 2 * sizeof(uint64_t) + sizeof(model::MetadataType)> key;
		std::iota(key.begin(), key.end(), static_cast<uint8_t>(1));
		key[0] = seed;
		key[key.size() - 1] = utils::to_underlying_type(model::MetadataType::Mosaic);

		Hash256 hash;
		crypto::Sha3_256(key, hash);
		return hash;
	}

	state::MetadataKey GenerateRandomMetadataKey() {
		auto partialKey = GenerateRandomPartialMetadataKey();
		auto targetId = Random();

		auto metadataType = static_cast<model::MetadataType>(Random() % 3);
		switch (metadataType) {
		case model::MetadataType::Account:
			return state::MetadataKey(partialKey);

		case model::MetadataType::Mosaic:
			return state::MetadataKey(partialKey, MosaicId(targetId));

		case model::MetadataType::Namespace:
			return state::MetadataKey(partialKey, NamespaceId(targetId));
		}

		CATAPULT_THROW_INVALID_ARGUMENT_1("cannot create MetadataKey with unexpected type", static_cast<uint16_t>(metadataType));
	}

	state::MetadataKey GenerateMetadataKey(const Hash256& hash) {
		// hack to set the metadata unique key (required for cache tests)
		auto key = GenerateRandomMetadataKey();
		const_cast<Hash256&>(key.uniqueKey()) = hash;
		return key;
	}

	// endregion

	// region notifications

	model::MetadataValueNotification CreateMetadataValueNotification(
			const state::MetadataKey& metadataKey,
			int16_t valueSizeDelta,
			uint16_t valueSize,
			const uint8_t* pValue) {
		auto targetId = metadataKey.targetId();
		if (model::MetadataType::Mosaic == metadataKey.metadataType())
			targetId = UnresolveXor(MosaicId(targetId)).unwrap();

		return model::MetadataValueNotification(
				{ metadataKey.sourceAddress(), UnresolveXor(metadataKey.targetAddress()), metadataKey.scopedMetadataKey() },
				{ metadataKey.metadataType(), targetId },
				valueSizeDelta,
				valueSize,
				pValue);
	}

	// endregion

	// region asserts

	namespace {
		void AssertEqual(const state::MetadataKey& expected, const state::MetadataKey& actual) {
			EXPECT_EQ(expected.metadataType(), actual.metadataType());
			EXPECT_EQ(expected.uniqueKey(), actual.uniqueKey());
		}

		void AssertEqual(const state::MetadataValue& expected, const state::MetadataValue& actual) {
			ASSERT_EQ(expected.size(), actual.size());
			EXPECT_EQ_MEMORY(expected.data(), actual.data(), expected.size());
		}
	}

	void AssertEqual(const state::MetadataEntry& expected, const state::MetadataEntry& actual) {
		AssertEqual(expected.key(), actual.key());
		AssertEqual(expected.value(), actual.value());
	}

	// endregion
}}
