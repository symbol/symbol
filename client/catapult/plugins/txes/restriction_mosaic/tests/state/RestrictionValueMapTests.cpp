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

#include "src/state/RestrictionValueMap.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS RestrictionValueMapTests

	// region ctor

	TEST(TEST_CLASS, CanCreateMap) {
		// Act:
		RestrictionValueMap<uint64_t> map;

		// Assert:
		EXPECT_EQ(0u, map.size());
		EXPECT_EQ(std::set<uint64_t>(), map.keys());
	}

	// endregion

	// region get

	TEST(TEST_CLASS, CannotGetValueForUnsetRestriction) {
		// Arrange:
		RestrictionValueMap<uint64_t> map;
		map.set(111, 444);

		// Act:
		uint64_t value;
		auto result = map.tryGet(112, value);

		// Assert:
		EXPECT_FALSE(result);
	}

	TEST(TEST_CLASS, CanGetValueForSetRestriction) {
		// Arrange:
		RestrictionValueMap<uint64_t> map;
		map.set(111, 444);

		// Act:
		uint64_t value;
		auto result = map.tryGet(111, value);

		// Assert:
		EXPECT_TRUE(result);
		EXPECT_EQ(444u, value);
	}

	// endregion

	// region set

	namespace {
		void AssertCanGetValue(const RestrictionValueMap<uint64_t>& map, uint64_t key, uint64_t expectedValue) {
			uint64_t value;
			auto result = map.tryGet(key, value);

			EXPECT_TRUE(result) << "for key " << key;
			EXPECT_EQ(expectedValue, value) << "for key " << key;
		}

		void AssertCannotGetValue(const RestrictionValueMap<uint64_t>& map, uint64_t key) {
			uint64_t value;
			auto result = map.tryGet(key, value);

			EXPECT_FALSE(result) << "for key " << key;
		}
	}

	TEST(TEST_CLASS, CanSetSingleValue) {
		// Arrange:
		RestrictionValueMap<uint64_t> map;

		// Act:
		map.set(111, 444);

		// Assert:
		EXPECT_EQ(1u, map.size());
		AssertCanGetValue(map, 111, 444);
		EXPECT_EQ(std::set<uint64_t>({ 111 }), map.keys());
	}

	TEST(TEST_CLASS, CanSetMultipleValues) {
		// Arrange:
		RestrictionValueMap<uint64_t> map;

		// Act:
		map.set(111, 444);
		map.set(321, 987);
		map.set(222, 567);

		// Assert:
		EXPECT_EQ(3u, map.size());
		AssertCanGetValue(map, 111, 444);
		AssertCanGetValue(map, 222, 567);
		AssertCanGetValue(map, 321, 987);
		EXPECT_EQ(std::set<uint64_t>({ 111, 222, 321 }), map.keys());
	}

	TEST(TEST_CLASS, CanChangeSingleValue) {
		// Arrange:
		RestrictionValueMap<uint64_t> map;
		map.set(111, 444);
		map.set(321, 987);

		// Act:
		map.set(111, 555);

		// Assert:
		EXPECT_EQ(2u, map.size());
		AssertCanGetValue(map, 111, 555);
		AssertCanGetValue(map, 321, 987);
		EXPECT_EQ(std::set<uint64_t>({ 111, 321 }), map.keys());
	}

	// endregion

	// region remove

	TEST(TEST_CLASS, CanRemoveValueNotInMap) {
		// Arrange:
		RestrictionValueMap<uint64_t> map;

		// Act:
		map.remove(111);

		// Assert:
		EXPECT_EQ(0u, map.size());
		AssertCannotGetValue(map, 111);
		EXPECT_EQ(std::set<uint64_t>(), map.keys());
	}

	TEST(TEST_CLASS, CanRemoveSingleValue) {
		// Arrange:
		RestrictionValueMap<uint64_t> map;
		map.set(111, 444);
		map.set(321, 987);

		// Act:
		map.remove(111);

		// Assert:
		EXPECT_EQ(1u, map.size());
		AssertCannotGetValue(map, 111);
		AssertCanGetValue(map, 321, 987);
		EXPECT_EQ(std::set<uint64_t>({ 321 }), map.keys());
	}

	// endregion
}}
