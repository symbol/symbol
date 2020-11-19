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

#include "catapult/utils/ByteArray.h"
#include "tests/test/nodeps/Comparison.h"
#include "tests/test/nodeps/Convertibility.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS ByteArrayTests

	namespace {
		struct TestArray_tag { static constexpr size_t Size = 8; };
		using TestArray = ByteArray<TestArray_tag>;

		using AliasedArray = TestArray;

		struct SameSizeArray_tag { static constexpr size_t Size = 8; };
		using SameSizeArray = ByteArray<SameSizeArray_tag>;

		// region ReadAll utils

		template<typename TByteArray>
		std::array<uint8_t, 8> ReadAllViaIndexOperator(TByteArray& array) {
			std::array<uint8_t, 8> readBytes{};
			for (auto i = 0u; i < array.size(); ++i)
				readBytes[i] = array[i];

			return readBytes;
		}

		template<typename TByteArray>
		std::array<uint8_t, 8> ReadAllViaDataPointer(TByteArray& array) {
			std::array<uint8_t, 8> readBytes{};
			for (auto i = 0u; i < array.size(); ++i)
				readBytes[i] = array.data()[i];

			return readBytes;
		}

		template<typename TByteArray>
		std::array<uint8_t, 8> ReadAllViaBeginEnd(TByteArray& array) {
			auto i = 0u;
			std::array<uint8_t, 8> readBytes{};
			for (auto byte : array)
				readBytes[i++] = byte;

			return readBytes;
		}

		template<typename TByteArray>
		std::array<uint8_t, 8> ReadAllViaCBeginCEnd(TByteArray& array) {
			auto i = 0u;
			std::array<uint8_t, 8> readBytes{};
			for (auto iter = array.cbegin(); array.cend() != iter; ++iter)
				readBytes[i++] = *iter;

			return readBytes;
		}

		void AssertContents(const std::array<uint8_t, 8>& expected, TestArray& array) {
			// Assert:
			EXPECT_EQ(8u, array.size());

			EXPECT_EQ(expected, ReadAllViaIndexOperator(array));
			EXPECT_EQ(expected, ReadAllViaIndexOperator<const TestArray>(array));

			EXPECT_EQ(expected, ReadAllViaDataPointer(array));
			EXPECT_EQ(expected, ReadAllViaDataPointer<const TestArray>(array));

			EXPECT_EQ(expected, ReadAllViaBeginEnd(array));
			EXPECT_EQ(expected, ReadAllViaBeginEnd<const TestArray>(array));

			EXPECT_EQ(expected, ReadAllViaCBeginCEnd(array));
			EXPECT_EQ(expected, ReadAllViaCBeginCEnd<const TestArray>(array));
		}

		// endregion
	}

	// region constructor

	TEST(TEST_CLASS, DefaultArrayIsZeroInitialized) {
		// Act:
		TestArray array;

		// Assert:
		AssertContents({ { 0, 0, 0, 0, 0, 0, 0, 0 } }, array);
	}

	TEST(TEST_CLASS, CanStoreArray) {
		// Arrange:
		TestArray array{ { 1, 2, 8, 7, 6, 2, 9, 0 } };

		// Act + Assert:
		AssertContents({ { 1, 2, 8, 7, 6, 2, 9, 0 } }, array);
	}

	// endregion

	// region copy

	TEST(TEST_CLASS, CanCopyAssign) {
		// Arrange:
		TestArray array{ { 1, 2, 8, 7, 6, 2, 9, 0 } };
		TestArray newArray{ { 5, 6, 7, 8, 0, 1, 2, 9 } };

		// Act:
		const auto& assignResult = (array = newArray);

		// Assert:
		AssertContents({ { 5, 6, 7, 8, 0, 1, 2, 9 } }, newArray);
		AssertContents({ { 5, 6, 7, 8, 0, 1, 2, 9 } }, array);
		EXPECT_EQ(&array, &assignResult);
	}

	TEST(TEST_CLASS, CanCopyConstruct) {
		// Act:
		TestArray array{ { 1, 2, 8, 7, 6, 2, 9, 0 } };
		TestArray newArray(array);

		// Assert:
		AssertContents({ { 1, 2, 8, 7, 6, 2, 9, 0 } }, newArray);
		AssertContents({ { 1, 2, 8, 7, 6, 2, 9, 0 } }, array);
	}

	// endregion

	// region comparison operators

	namespace {
		std::vector<TestArray> GenerateIncreasingArrays() {
			return {
				TestArray{ { 1, 1, 1, 1, 1, 1, 1, 1 } },
				TestArray{ { 2, 2, 2, 2, 1, 1, 1, 1 } },
				TestArray{ { 2, 2, 2, 2, 1, 1, 1, 2 } }
			};
		}
	}

	DEFINE_EQUALITY_AND_COMPARISON_TESTS(TEST_CLASS, GenerateIncreasingArrays())

	// endregion

	// region to string

	TEST(TEST_CLASS, CanOutputByteArray) {
		// Arrange:
		TestArray array{ { 1, 2, 8, 7, 6, 2, 9, 0 } };

		// Act:
		auto str = test::ToString(array);

		// Assert:
		EXPECT_EQ("0102080706020900", str);
	}

	// endregion

	// region copyTo

	TEST(TEST_CLASS, CopyToSupportsSameSizeDestinationByteArray) {
		// Arrange:
		auto sourceArray = test::GenerateRandomByteArray<Key>();

		// Act:
		auto destArray = sourceArray.copyTo<Hash256>();

		// Assert:
		EXPECT_EQ(sourceArray.size(), destArray.size());
		EXPECT_EQ_MEMORY(sourceArray.data(), destArray.data(), Hash256::Size);
	}

	TEST(TEST_CLASS, CopyToSupportsSmallerSizeDestinationByteArray) {
		// Arrange:
		auto sourceArray = test::GenerateRandomByteArray<Hash512>();

		// Act:
		auto destArray = sourceArray.copyTo<Hash256>();

		// Assert:
		EXPECT_GT(sourceArray.size(), destArray.size());
		EXPECT_EQ_MEMORY(sourceArray.data(), destArray.data(), Hash256::Size);
	}

	TEST(TEST_CLASS, CopyToSupportsLargerSizeDestinationByteArray) {
		// Arrange:
		auto sourceArray = test::GenerateRandomByteArray<Hash256>();

		// Act:
		auto destArray = sourceArray.copyTo<Hash512>();

		// Assert:
		EXPECT_LT(sourceArray.size(), destArray.size());
		EXPECT_EQ_MEMORY(sourceArray.data(), destArray.data(), Hash256::Size);
		EXPECT_EQ_MEMORY(Hash256().data(), destArray.data() + Hash256::Size, Hash256::Size);
	}

	// endregion

	// region type convertibility

	TEST(TEST_CLASS, CanAssignAliasedType) {
		auto isConvertible = std::is_convertible_v<TestArray, AliasedArray>;
		EXPECT_TRUE(isConvertible);
	}

	TEST(TEST_CLASS, CannotAssignUsingDifferentType) {
		auto isConvertible = std::is_convertible_v<TestArray, SameSizeArray>;
		EXPECT_FALSE(isConvertible);
	}

	TEST(TEST_CLASS, CatapultTypesTests) {
		test::TypeConvertibilityTests::AssertCannotConvertTypes<Signature, Hash512>();
		test::TypeConvertibilityTests::AssertCannotConvertTypes<Key, Hash256, GenerationHash>();
	}

	// endregion
}}
