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

#pragma once
#include "src/model/MetadataNotifications.h"
#include "src/state/MetadataKey.h"

namespace catapult { namespace state { class MetadataEntry; } }

namespace catapult { namespace test {

	// region PartialMetadataKey

	/// Generates a random partial metadata key.
	model::PartialMetadataKey GenerateRandomPartialMetadataKey();

	/// Generates a random partial metadata key with \a scopedMetadataKey.
	model::PartialMetadataKey GenerateRandomPartialMetadataKey(uint64_t scopedMetadataKey);

	// endregion

	// region MetadataKey

	/// Creates a metadata unique key from \a seed.
	Hash256 CreateMetadataUniqueKeyFromSeed(uint8_t seed);

	/// Generates a random metadata key.
	state::MetadataKey GenerateRandomMetadataKey();

	/// Generates a metadata key with \a hash.
	state::MetadataKey GenerateMetadataKey(const Hash256& hash);

	// endregion

	// region notifications

	/// Creates a metadata value notification around \a metadataKey, \a valueSizeDelta, \a valueSize and \a pValue
	model::MetadataValueNotification CreateMetadataValueNotification(
			const state::MetadataKey& metadataKey,
			int16_t valueSizeDelta,
			uint16_t valueSize,
			const uint8_t* pValue);

	// endregion

	// region asserts

	/// Asserts that metadata entry \a actual is equal to \a expected.
	void AssertEqual(const state::MetadataEntry& expected, const state::MetadataEntry& actual);

	// endregion
}}
