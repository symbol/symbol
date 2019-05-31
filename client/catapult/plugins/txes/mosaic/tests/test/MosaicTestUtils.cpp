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

	model::MosaicProperties CreateMosaicPropertiesWithDuration(BlockDuration duration) {
		model::MosaicProperties::PropertyValuesContainer values{};
		values[utils::to_underlying_type(model::MosaicPropertyId::Duration)] = duration.unwrap();
		return model::MosaicProperties::FromValues(values);
	}

	state::MosaicDefinition CreateMosaicDefinition(Height height) {
		return state::MosaicDefinition(height, test::GenerateRandomByteArray<Key>(), 3, model::MosaicProperties::FromValues({}));
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
		state::MosaicDefinition CreateMosaicDefinition(Height height, const Key& owner, BlockDuration duration) {
			return state::MosaicDefinition(height, owner, 3, CreateMosaicPropertiesWithDuration(duration));
		}
	}

	state::MosaicEntry CreateMosaicEntry(MosaicId id, Height height, const Key& owner, Amount supply, BlockDuration duration) {
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
			EXPECT_EQ(expected.height(), actual.height());
			EXPECT_EQ(expected.owner(), actual.owner());
			AssertMosaicDefinitionProperties(expected.properties(), actual.properties());
		}
	}

	void AssertEqual(const state::MosaicEntry& expected, const state::MosaicEntry& actual) {
		EXPECT_EQ(expected.mosaicId(), actual.mosaicId());
		EXPECT_EQ(expected.supply(), actual.supply());
		AssertEqual(expected.definition(), actual.definition());
	}
}}
