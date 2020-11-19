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

#include "catapult/utils/Hashers.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS HashersTests

	// region ArrayHasher

	namespace {
		struct TestArray_tag { static constexpr size_t Size = 4 + sizeof(size_t); };
		using TestArray = ByteArray<TestArray_tag>;
	}

	TEST(TEST_CLASS, Array_SameObjectReturnsSameHash) {
		// Arrange:
		ArrayHasher<TestArray> hasher;
		auto array1 = test::GenerateRandomByteArray<TestArray>();

		// Act:
		auto result1 = hasher(array1);
		auto result2 = hasher(array1);

		// Assert:
		EXPECT_EQ(result1, result2);
	}

	TEST(TEST_CLASS, Array_EqualObjectsReturnSameHash) {
		// Arrange:
		ArrayHasher<TestArray> hasher;
		auto array1 = test::GenerateRandomByteArray<TestArray>();
		auto array2 = array1;

		// Act:
		auto result1 = hasher(array1);
		auto result2 = hasher(array2);

		// Assert:
		EXPECT_EQ(result1, result2);
	}

	TEST(TEST_CLASS, Array_DifferentObjectsReturnDifferentHashes) {
		// Arrange:
		ArrayHasher<TestArray> hasher;
		auto array1 = test::GenerateRandomByteArray<TestArray>();
		TestArray array2;
		std::transform(array1.cbegin(), array1.cend(), array2.begin(), [](auto byte) { return static_cast<uint8_t>(byte ^ 0xFF); });

		// Act:
		auto result1 = hasher(array1);
		auto result2 = hasher(array2);

		// Assert:
		EXPECT_NE(result1, result2);
	}

	TEST(TEST_CLASS, Array_DifferentHasherOffsetsReturnDifferentHashes) {
		// Arrange:
		auto array1 = test::GenerateRandomByteArray<TestArray>();

		// Act:
		auto result1 = ArrayHasher<TestArray, 0>()(array1);
		auto result2 = ArrayHasher<TestArray, 4>()(array1);

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
