#include "catapult/utils/Array.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"
#include <unordered_map>
#include <unordered_set>

namespace catapult { namespace utils {

	namespace {
		constexpr size_t Default_Size = 10;

		using UintArray = Array<size_t, Default_Size>;
	}

	// region ctor

	TEST(ArrayTests, CanCreateArray) {
		// Act:
		UintArray array;

		// Assert:
		EXPECT_EQ(Default_Size, array.capacity());
		EXPECT_TRUE(array.empty());
		EXPECT_EQ(0u, array.size());
	}

	// endregion

	// region push_back

	TEST(ArrayTests, CanAddSingleElement) {
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

	TEST(ArrayTests, CanAddMultipleElements) {
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

	TEST(ArrayTests, AddingElementThrowsIfCapacityIsExceeded) {
		// Arrange:
		UintArray array;
		for (auto i = 0u; i < Default_Size; ++i)
			array.push_back(i);

		// Sanity:
		EXPECT_EQ(array.capacity(), array.size());

		// Assert:
		EXPECT_THROW(array.push_back(123), catapult_out_of_range);
	}

	// endregion

	// region operator[]

	TEST(ArrayTests, SubscriptOperatorReturnsExpectedElement) {
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

	TEST(ArrayTests, SubscriptOperatorThrowsIfIndexIsOutOfRange) {
		// Arrange:
		UintArray array;
		for (auto i = 0u; i < Default_Size / 2; ++i)
			array.push_back(i * i);

		// Sanity:
		EXPECT_EQ(Default_Size / 2, array.size());

		// Assert:
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

	TEST(ArrayTests, OperatorEqualReturnsTrueForEqualObjects) {
		// Assert:
		test::AssertOperatorEqualReturnsTrueForEqualObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(ArrayTests, OperatorNotEqualReturnsTrueForUnequalObjects) {
		// Assert:
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	// endregion
}}
