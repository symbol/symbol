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

#include "catapult/utils/CheckedArray.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/test/nodeps/IteratorTestTraits.h"
#include "tests/TestHarness.h"
#include <unordered_map>
#include <unordered_set>

namespace catapult { namespace utils {

#define TEST_CLASS CheckedArrayTests

	namespace {
		constexpr size_t Default_Size = 10;

		using UintCheckedArray = CheckedArray<size_t, Default_Size>;
	}

	// region ctor

	TEST(TEST_CLASS, CanCreateArray) {
		// Act:
		UintCheckedArray array;

		// Assert:
		EXPECT_EQ(Default_Size, array.capacity());
		EXPECT_TRUE(array.empty());
		EXPECT_EQ(0u, array.size());
	}

	// endregion

	// region push_back

	TEST(TEST_CLASS, CanAddSingleElement) {
		// Arrange:
		UintCheckedArray array;

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
		UintCheckedArray array;

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

	TEST(TEST_CLASS, AddingElementThrowsWhenCapacityIsExceeded) {
		// Arrange:
		UintCheckedArray array;
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
		UintCheckedArray array;
		for (auto i = 0u; i < Default_Size; ++i)
			array.push_back(i * i);

		// Sanity:
		EXPECT_EQ(array.capacity(), array.size());

		// Assert:
		for (auto i = 0u; i < Default_Size; ++i)
			EXPECT_EQ(i * i, array[i]) << "element at " << i;
	}

	TEST(TEST_CLASS, SubscriptOperatorThrowsWhenIndexIsOutOfRange) {
		// Arrange:
		UintCheckedArray array;
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

	// region iterators

	namespace {
		template<typename TTraits>
		void AssertIteratorsReturnProperRepresentation(size_t numElements) {
			// Arrange:
			UintCheckedArray array;
			for (auto i = 0u; i < numElements; ++i)
				array.push_back(i * i);

			// Sanity:
			EXPECT_GE(array.capacity(), array.size()) << " size: " << numElements;

			// Act:
			auto count = static_cast<size_t>(std::distance(TTraits::begin(array), TTraits::end(array)));

			// Assert: end points past last element, check all elements
			EXPECT_EQ(numElements, count);
			EXPECT_EQ(&array[array.size() - 1] + 1, TTraits::end(array)) << " size: " << numElements;

			size_t i = 0;
			for (auto iter = TTraits::begin(array); TTraits::end(array) != iter; ++iter, ++i)
				EXPECT_EQ(&array[i], iter) << " size: " << numElements << ", element at " << i;
		}

#define ITERATOR_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_BeginEnd) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::BeginEndConstTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_CBeginCEnd) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<test::CBeginCEndTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()
	}

	ITERATOR_BASED_TEST(IteratorsReturnProperRepresentation_EmptyArray) {
		// Arrange:
		UintCheckedArray array;

		// Act:
		auto count = std::distance(TTraits::begin(array), TTraits::end(array));

		// Assert:
		EXPECT_EQ(0u, count);
		EXPECT_EQ(TTraits::begin(array), TTraits::end(array));
	}

	ITERATOR_BASED_TEST(IteratorsReturnProperRepresentation_ArrayWithValues) {
		for (auto i = 1u; i <= Default_Size; ++i)
			AssertIteratorsReturnProperRepresentation<TTraits>(i);
	}

	// endregion

	// region equality

	namespace {
		const char* Default_Key = "default";

		auto CreateUintCheckedArray(
				size_t numElements,
				const std::function<size_t (size_t value)>& createElement = [](auto value) { return value * value; }) {
			UintCheckedArray array;
			for (auto i = 0u; i < numElements; ++i)
				array.push_back(createElement(i));

			return array;
		}

		auto GenerateEqualityInstanceMap() {
			std::unordered_map<std::string, UintCheckedArray> map;

			map.emplace(Default_Key, CreateUintCheckedArray(Default_Size / 2));
			map.emplace("copy", CreateUintCheckedArray(Default_Size / 2));
			map.emplace("diff-more-elements", CreateUintCheckedArray(Default_Size / 2 + 1));
			map.emplace("diff-less-elements", CreateUintCheckedArray(Default_Size / 2 - 1));
			map.emplace("diff-unequal-elements", CreateUintCheckedArray(Default_Size / 2, [](auto value) { return value; }));
			return map;
		}

		std::unordered_set<std::string> GetEqualTags() {
			return { Default_Key, "copy" };
		}
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueForEqualObjects) {
		test::AssertOperatorEqualReturnsTrueForEqualObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueForUnequalObjects) {
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	// endregion
}}
