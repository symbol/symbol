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

#include "catapult/utils/Array.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"
#include <unordered_map>
#include <unordered_set>

namespace catapult { namespace utils {

#define TEST_CLASS ArrayTests

	namespace {
		constexpr size_t Default_Size = 10;

		using UintArray = Array<size_t, Default_Size>;
	}

	// region ctor

	TEST(TEST_CLASS, CanCreateArray) {
		// Act:
		UintArray array;

		// Assert:
		EXPECT_EQ(Default_Size, array.capacity());
		EXPECT_TRUE(array.empty());
		EXPECT_EQ(0u, array.size());
	}

	// endregion

	// region push_back

	TEST(TEST_CLASS, CanAddSingleElement) {
		// Arrange:
		UintArray array;

		// Act:
		array.push_back(3);

		// Assert:
		EXPECT_EQ(Default_Size, array.capacity());
		EXPECT_FALSE(array.empty());
		EXPECT_EQ(1u, array.size());
		EXPECT_EQ(3u, array[0]);
	}

	TEST(TEST_CLASS, CanAddMultipleElements) {
		// Arrange:
		UintArray array;

		// Act:
		array.push_back(3);
		array.push_back(13);
		array.push_back(2357);

		// Assert:
		EXPECT_EQ(Default_Size, array.capacity());
		EXPECT_FALSE(array.empty());
		EXPECT_EQ(3u, array.size());
		EXPECT_EQ(3u, array[0]);
		EXPECT_EQ(13u, array[1]);
		EXPECT_EQ(2357u, array[2]);
	}

	TEST(TEST_CLASS, AddingElementThrowsIfCapacityIsExceeded) {
		// Arrange:
		UintArray array;
		for (auto i = 0u; i < Default_Size; ++i)
			array.push_back(i);

		// Sanity:
		EXPECT_EQ(array.capacity(), array.size());

		// Act + Assert:
		EXPECT_THROW(array.push_back(123), catapult_out_of_range);
	}

	// endregion

	// region operator[]

	TEST(TEST_CLASS, SubscriptOperatorReturnsExpectedElement) {
		// Arrange:
		UintArray array;
		for (auto i = 0u; i < Default_Size; ++i)
			array.push_back(i * i);

		// Sanity:
		EXPECT_EQ(array.capacity(), array.size());

		// Assert:
		for (auto i = 0u; i < Default_Size; ++i)
			EXPECT_EQ(i * i, array[i]) << "element at " << i;
	}

	TEST(TEST_CLASS, SubscriptOperatorThrowsIfIndexIsOutOfRange) {
		// Arrange:
		UintArray array;
		for (auto i = 0u; i < Default_Size / 2; ++i)
			array.push_back(i * i);

		// Sanity:
		EXPECT_EQ(Default_Size / 2, array.size());

		// Act + Assert:
		EXPECT_THROW(array[Default_Size / 2], catapult_out_of_range);
		EXPECT_THROW(array[Default_Size / 2 + 1], catapult_out_of_range);
		EXPECT_THROW(array[Default_Size], catapult_out_of_range);
		EXPECT_THROW(array[12345], catapult_out_of_range);
	}

	// endregion

	// region equality

	namespace {
		const char* Default_Key = "default";

		auto CreateArray(
				size_t numElements,
				const std::function<size_t (size_t value)>& createElement = [](auto value) { return value * value; }) {
			UintArray array;
			for (auto i = 0u; i < numElements; ++i)
				array.push_back(createElement(i));

			return array;
		}

		auto GenerateEqualityInstanceMap() {
			std::unordered_map<std::string, UintArray> map;

			map.emplace(Default_Key, CreateArray(Default_Size / 2));
			map.emplace("copy", CreateArray(Default_Size / 2));
			map.emplace("diff-more-elements", CreateArray(Default_Size / 2 + 1));
			map.emplace("diff-less-elements", CreateArray(Default_Size / 2 - 1));
			map.emplace("diff-unequal-elements", CreateArray(Default_Size / 2, [](auto value) { return value; }));
			return map;
		}

		std::unordered_set<std::string> GetEqualTags() {
			return { Default_Key, "copy" };
		}
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueForEqualObjects) {
		// Assert:
		test::AssertOperatorEqualReturnsTrueForEqualObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueForUnequalObjects) {
		// Assert:
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	// endregion
}}
