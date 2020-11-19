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

#include "src/state/MosaicAddressRestriction.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS MosaicAddressRestrictionTests

	// region ctor

	TEST(TEST_CLASS, CanCreateRestriction) {
		// Act:
		MosaicAddressRestriction restriction(MosaicId(123), Address{ { 35 } });

		// Assert:
		EXPECT_EQ(MosaicId(123), restriction.mosaicId());
		EXPECT_EQ(Address{ { 35 } }, restriction.address());

		EXPECT_EQ(0u, restriction.size());
		EXPECT_EQ(std::set<uint64_t>(), restriction.keys());
	}

	// endregion

	// region get

	TEST(TEST_CLASS, CannotGetValueForUnsetRestriction) {
		// Arrange:
		MosaicAddressRestriction restriction(MosaicId(123), Address{ { 35 } });
		restriction.set(111, 444);

		// Act:
		auto result = restriction.get(112);

		// Assert:
		EXPECT_EQ(MosaicAddressRestriction::Sentinel_Removal_Value, result);
	}

	TEST(TEST_CLASS, CanGetValueForSetRestriction) {
		// Arrange:
		MosaicAddressRestriction restriction(MosaicId(123), Address{ { 35 } });
		restriction.set(111, 444);

		// Act:
		auto result = restriction.get(111);

		// Assert:
		EXPECT_EQ(444u, result);
	}

	// endregion

	// region set

	TEST(TEST_CLASS, CanSetSingleValue) {
		// Arrange:
		MosaicAddressRestriction restriction(MosaicId(123), Address{ { 35 } });

		// Act:
		restriction.set(111, 444);

		// Assert:
		EXPECT_EQ(1u, restriction.size());
		EXPECT_EQ(444u, restriction.get(111));
		EXPECT_EQ(std::set<uint64_t>({ 111 }), restriction.keys());
	}

	TEST(TEST_CLASS, CannotSetSentinelValue) {
		// Arrange:
		MosaicAddressRestriction restriction(MosaicId(123), Address{ { 35 } });

		// Act:
		restriction.set(111, MosaicAddressRestriction::Sentinel_Removal_Value);

		// Assert:
		EXPECT_EQ(0u, restriction.size());
		EXPECT_EQ(MosaicAddressRestriction::Sentinel_Removal_Value, restriction.get(111));
		EXPECT_EQ(std::set<uint64_t>(), restriction.keys());
	}

	TEST(TEST_CLASS, CanSetMultipleValues) {
		// Arrange:
		MosaicAddressRestriction restriction(MosaicId(123), Address{ { 35 } });

		// Act:
		restriction.set(111, 444);
		restriction.set(321, 987);
		restriction.set(222, 567);

		// Assert:
		EXPECT_EQ(3u, restriction.size());
		EXPECT_EQ(444u, restriction.get(111));
		EXPECT_EQ(567u, restriction.get(222));
		EXPECT_EQ(987u, restriction.get(321));
		EXPECT_EQ(std::set<uint64_t>({ 111, 222, 321 }), restriction.keys());
	}

	TEST(TEST_CLASS, CanChangeSingleValue) {
		// Arrange:
		MosaicAddressRestriction restriction(MosaicId(123), Address{ { 35 } });
		restriction.set(111, 444);
		restriction.set(321, 987);

		// Act:
		restriction.set(111, 555);

		// Assert:
		EXPECT_EQ(2u, restriction.size());
		EXPECT_EQ(555u, restriction.get(111));
		EXPECT_EQ(987u, restriction.get(321));
		EXPECT_EQ(std::set<uint64_t>({ 111, 321 }), restriction.keys());
	}

	TEST(TEST_CLASS, CanRemoveSingleValue) {
		// Arrange:
		MosaicAddressRestriction restriction(MosaicId(123), Address{ { 35 } });
		restriction.set(111, 444);
		restriction.set(321, 987);

		// Act:
		restriction.set(111, MosaicAddressRestriction::Sentinel_Removal_Value);

		// Assert:
		EXPECT_EQ(1u, restriction.size());
		EXPECT_EQ(MosaicAddressRestriction::Sentinel_Removal_Value, restriction.get(111));
		EXPECT_EQ(987u, restriction.get(321));
		EXPECT_EQ(std::set<uint64_t>({ 321 }), restriction.keys());
	}

	// endregion
}}
