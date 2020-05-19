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

#include "src/state/MosaicEntry.h"
#include "tests/test/MosaicTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS MosaicEntryTests

	// region ctor

	TEST(TEST_CLASS, CanCreateMosaicEntry) {
		// Arrange:
		auto definition = test::CreateMosaicDefinition(Height(123));

		// Act:
		auto entry = MosaicEntry(MosaicId(225), definition);

		// Assert:
		EXPECT_EQ(MosaicId(225), entry.mosaicId());
		EXPECT_EQ(Height(123), entry.definition().startHeight());
		EXPECT_EQ(Amount(), entry.supply());
	}

	// endregion

	// region supply

	TEST(TEST_CLASS, CanIncreaseSupply) {
		// Arrange:
		auto definition = test::CreateMosaicDefinition(Height(123));
		auto entry = MosaicEntry(MosaicId(225), definition);
		entry.increaseSupply(Amount(432));

		// Act:
		entry.increaseSupply(Amount(321));

		// Assert:
		EXPECT_EQ(Amount(432 + 321), entry.supply());
	}

	TEST(TEST_CLASS, CanDecreaseSupply) {
		// Arrange:
		auto definition = test::CreateMosaicDefinition(Height(123));
		auto entry = MosaicEntry(MosaicId(225), definition);
		entry.increaseSupply(Amount(432));

		// Act:
		entry.decreaseSupply(Amount(321));

		// Assert:
		EXPECT_EQ(Amount(432 - 321), entry.supply());
	}

	TEST(TEST_CLASS, CanDecreaseSupplyToZero) {
		// Arrange:
		auto definition = test::CreateMosaicDefinition(Height(123));
		auto entry = MosaicEntry(MosaicId(225), definition);
		entry.increaseSupply(Amount(432));

		// Act:
		entry.decreaseSupply(Amount(432));

		// Assert:
		EXPECT_EQ(Amount(), entry.supply());
	}

	TEST(TEST_CLASS, CannotDecreaseSupplyBelowZero) {
		// Arrange:
		auto definition = test::CreateMosaicDefinition(Height(123));
		auto entry = MosaicEntry(MosaicId(225), definition);
		entry.increaseSupply(Amount(432));

		// Act + Assert:
		EXPECT_THROW(entry.decreaseSupply(Amount(433)), catapult_invalid_argument);
	}

	// endregion

	// region isActive

	namespace {
		MosaicDefinition CreateMosaicDefinition(Height height, uint64_t duration) {
			auto owner = test::CreateRandomOwner();
			return MosaicDefinition(height, owner, 3, test::CreateMosaicPropertiesWithDuration(BlockDuration(duration)));
		}
	}

	TEST(TEST_CLASS, IsActiveReturnsFalseWhenDefinitionIsInactive) {
		// Arrange: definition expires at height 123 + 150 = 273
		auto entry = MosaicEntry(MosaicId(225), CreateMosaicDefinition(Height(123), 150));

		// Act + Assert:
		EXPECT_FALSE(entry.isActive(Height(50)));
		EXPECT_FALSE(entry.isActive(Height(122)));
		EXPECT_FALSE(entry.isActive(Height(273)));
		EXPECT_FALSE(entry.isActive(Height(350)));
	}

	TEST(TEST_CLASS, IsActiveReturnsTrueWhenDefinitionIsActive) {
		// Arrange: definition expires at height 123 + 150 = 273
		auto entry = MosaicEntry(MosaicId(225), CreateMosaicDefinition(Height(123), 150));

		// Act + Assert:
		EXPECT_TRUE(entry.isActive(Height(123)));
		EXPECT_TRUE(entry.isActive(Height(250)));
		EXPECT_TRUE(entry.isActive(Height(272)));
	}

	// endregion
}}
