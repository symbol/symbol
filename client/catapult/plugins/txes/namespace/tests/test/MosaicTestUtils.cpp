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
		return state::MosaicDefinition(height, test::GenerateRandomData<Key_Size>(), model::MosaicProperties::FromValues({}));
	}

	state::MosaicEntry CreateMosaicEntry(MosaicId id, Amount supply) {
		return CreateMosaicEntry(id, Height(987), supply);
	}

	state::MosaicEntry CreateMosaicEntry(MosaicId id, Height height, Amount supply) {
		auto entry = state::MosaicEntry(NamespaceId(234), id, CreateMosaicDefinition(height));
		entry.increaseSupply(supply);
		return entry;
	}

	namespace {
		state::MosaicDefinition CreateMosaicDefinition(Height height, const Key& owner, BlockDuration duration) {
			return state::MosaicDefinition(height, owner, CreateMosaicPropertiesWithDuration(duration));
		}
	}

	std::shared_ptr<const state::MosaicEntry> CreateMosaicEntry(
			NamespaceId namespaceId,
			MosaicId id,
			Height height,
			const Key& owner,
			Amount supply,
			BlockDuration duration) {
		auto pEntry = std::make_shared<state::MosaicEntry>(namespaceId, id, CreateMosaicDefinition(height, owner, duration));
		pEntry->increaseSupply(supply);
		return pEntry;
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
}}
