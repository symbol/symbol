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
#include "src/state/MosaicEntry.h"

namespace catapult { namespace test {

	/// Creates a random mosaic owner.
	Address CreateRandomOwner();

	/// Creates mosaic properties from values: \a flags, \a divisibility, \a duration.
	model::MosaicProperties CreateMosaicPropertiesFromValues(uint8_t flags, uint8_t divisibility, uint64_t duration);

	/// Creates mosaic properties with a custom \a duration.
	model::MosaicProperties CreateMosaicPropertiesWithDuration(BlockDuration duration);

	/// Creates a mosaic definition with \a height.
	state::MosaicDefinition CreateMosaicDefinition(Height height);

	/// Creates a mosaic entry with \a id and \a supply.
	state::MosaicEntry CreateMosaicEntry(MosaicId id, Amount supply);

	/// Creates a mosaic entry with \a id, \a height and \a supply.
	state::MosaicEntry CreateMosaicEntry(MosaicId id, Height height, Amount supply);

	/// Creates a mosaic entry around \a id, \a height, \a owner, \a supply and \a duration.
	state::MosaicEntry CreateMosaicEntry(MosaicId id, Height height, const Address& owner, Amount supply, BlockDuration duration);

	/// Asserts that actual properties (\a actualProperties) exactly match expected properties (\a expectedProperties).
	void AssertMosaicDefinitionProperties(
			const model::MosaicProperties& expectedProperties,
			const model::MosaicProperties& actualProperties);

	/// Asserts that mosaic entry \a actual is equal to \a expected.
	void AssertEqual(const state::MosaicEntry& expected, const state::MosaicEntry& actual);
}}
