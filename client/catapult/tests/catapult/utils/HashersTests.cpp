#include "catapult/utils/Hashers.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS HashersTests

	// region ArrayHasher

	namespace {
		using TestArray = std::array<uint8_t, 4 + sizeof(size_t)>;
	}

	TEST(TEST_CLASS, Array_SameObjectReturnsSameHash) {
		// Arrange:
		ArrayHasher<TestArray> hasher;
		auto data1 = test::GenerateRandomData<sizeof(TestArray)>();

		// Act:
		auto result1 = hasher(data1);
		auto result2 = hasher(data1);

		// Assert:
		EXPECT_EQ(result1, result2);
	}

	TEST(TEST_CLASS, Array_EqualObjectsReturnSameHash) {
		// Arrange:
		ArrayHasher<TestArray> hasher;
		auto data1 = test::GenerateRandomData<sizeof(TestArray)>();
		auto data2 = data1;

		// Act:
		auto result1 = hasher(data1);
		auto result2 = hasher(data2);

		// Assert:
		EXPECT_EQ(result1, result2);
	}

	TEST(TEST_CLASS, Array_DifferentObjectsReturnDifferentHashes) {
		// Arrange:
		ArrayHasher<TestArray> hasher;
		auto data1 = test::GenerateRandomData<sizeof(TestArray)>();
		TestArray data2;
		std::transform(data1.cbegin(), data1.cend(), data2.begin(), [](auto byte) { return static_cast<uint8_t>(byte ^ 0xFF); });

		// Act:
		auto result1 = hasher(data1);
		auto result2 = hasher(data2);

		// Assert:
		EXPECT_NE(result1, result2);
	}

	TEST(TEST_CLASS, Array_DifferentHasherOffsetsReturnDifferentHashes) {
		// Arrange:
		auto data1 = test::GenerateRandomData<sizeof(TestArray)>();

		// Act:
		auto result1 = ArrayHasher<TestArray, 0>()(data1);
		auto result2 = ArrayHasher<TestArray, 4>()(data1);

		// Assert:
		EXPECT_NE(result1, result2);
	}

	// endregion

	// region BaseValueHasher

	TEST(TEST_CLASS, BaseValue_SameObjectReturnsSameHash) {
		// Arrange:
		BaseValueHasher<Amount> hasher;
		auto amount1 = Amount(7);

		// Act:
		auto result1 = hasher(amount1);
		auto result2 = hasher(amount1);

		// Assert:
		EXPECT_EQ(result1, result2);
	}

	TEST(TEST_CLASS, BaseValue_EqualObjectsReturnSameHash) {
		// Arrange:
		BaseValueHasher<Amount> hasher;
		auto amount1 = Amount(7);
		auto amount2 = amount1;

		// Act:
		auto result1 = hasher(amount1);
		auto result2 = hasher(amount2);

		// Assert:
		EXPECT_EQ(result1, result2);
	}

	TEST(TEST_CLASS, BaseValue_DifferentObjectsReturnDifferentHashes) {
		// Arrange:
		BaseValueHasher<Amount> hasher;
		auto amount1 = Amount(7);
		auto amount2 = amount1 + Amount(1);

		// Act:
		auto result1 = hasher(amount1);
		auto result2 = hasher(amount2);

		// Assert:
		EXPECT_NE(result1, result2);
	}

	// endregion
}}
