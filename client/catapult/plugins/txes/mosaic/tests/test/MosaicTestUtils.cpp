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

#include "MosaicTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	Address CreateRandomOwner() {
		return GenerateRandomByteArray<Address>();
	}

	model::MosaicProperties CreateMosaicPropertiesFromValues(uint8_t flags, uint8_t divisibility, uint64_t duration) {
		return model::MosaicProperties(static_cast<model::MosaicFlags>(flags), divisibility, BlockDuration(duration));
	}

	model::MosaicProperties CreateMosaicPropertiesWithDuration(BlockDuration duration) {
		return CreateMosaicPropertiesFromValues(0, 0, duration.unwrap());
	}

	state::MosaicDefinition CreateMosaicDefinition(Height height) {
		return state::MosaicDefinition(height, CreateRandomOwner(), 3, model::MosaicProperties());
	}

	state::MosaicEntry CreateMosaicEntry(MosaicId id, Amount supply) {
		return CreateMosaicEntry(id, Height(987), supply);
	}

	state::MosaicEntry CreateMosaicEntry(MosaicId id, Height height, Amount supply) {
		auto entry = state::MosaicEntry(id, CreateMosaicDefinition(height));
		entry.increaseSupply(supply);
		return entry;
	}

	namespace {
		state::MosaicDefinition CreateMosaicDefinition(Height height, const Address& owner, BlockDuration duration) {
			return state::MosaicDefinition(height, owner, 3, CreateMosaicPropertiesWithDuration(duration));
		}
	}

	state::MosaicEntry CreateMosaicEntry(MosaicId id, Height height, const Address& owner, Amount supply, BlockDuration duration) {
		auto entry = state::MosaicEntry(id, CreateMosaicDefinition(height, owner, duration));
		entry.increaseSupply(supply);
		return entry;
	}

	namespace {
		auto ToMosaicFlag(uint8_t value) {
			return static_cast<model::MosaicFlags>(1 << value);
		}
	}

	void AssertMosaicDefinitionProperties(
			const model::MosaicProperties& expectedProperties,
			const model::MosaicProperties& actualProperties) {
		for (uint8_t i = 0u; i < 8; ++i)
			EXPECT_EQ(expectedProperties.is(ToMosaicFlag(i)), actualProperties.is(ToMosaicFlag(i))) << "bit " << i;

		EXPECT_EQ(expectedProperties.divisibility(), actualProperties.divisibility());
		EXPECT_EQ(expectedProperties.duration(), actualProperties.duration());
	}

	namespace {
		void AssertEqual(const state::MosaicDefinition& expected, const state::MosaicDefinition& actual) {
			EXPECT_EQ(expected.startHeight(), actual.startHeight());
			EXPECT_EQ(expected.ownerAddress(), actual.ownerAddress());
			AssertMosaicDefinitionProperties(expected.properties(), actual.properties());
		}
	}

	void AssertEqual(const state::MosaicEntry& expected, const state::MosaicEntry& actual) {
		EXPECT_EQ(expected.mosaicId(), actual.mosaicId());
		EXPECT_EQ(expected.supply(), actual.supply());
		AssertEqual(expected.definition(), actual.definition());
	}
}}
