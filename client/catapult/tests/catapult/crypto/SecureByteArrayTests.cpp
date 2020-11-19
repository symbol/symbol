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

#include "catapult/crypto/SecureByteArray.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace crypto {

#define TEST_CLASS SecureByteArrayTests

	namespace {
		// region test utils

		struct ConcreteSecureByteArray_tag { static constexpr size_t Size = 24; };
		using ConcreteSecureByteArray = SecureByteArray<ConcreteSecureByteArray_tag>;

		using ConcreteByteArray = utils::ByteArray<ConcreteSecureByteArray_tag>;

		constexpr auto Zero = ConcreteByteArray();

		bool IsZero(const uint8_t* pMemory) {
			return std::equal(Zero.cbegin(), Zero.cend(), pMemory);
		}

		ConcreteSecureByteArray MakeSecure(const ConcreteByteArray& byteArray) {
			return ConcreteSecureByteArray::FromBuffer(byteArray);
		}

		void AssertEqual(const ConcreteByteArray& expected, const ConcreteSecureByteArray& actual) {
			EXPECT_TRUE(std::equal(expected.begin(), expected.end(), actual.data(), actual.data() + actual.size())); // raw data
			EXPECT_TRUE(std::equal(expected.begin(), expected.end(), actual.begin(), actual.end())); // const iterator

			EXPECT_EQ_MEMORY(expected.data(), actual.data(), expected.size()); // nicer error output
		}

		// endregion
	}

	// region constructor / destructor

	TEST(TEST_CLASS, CanCreateDefaultByteArray) {
		// Act:
		auto byteArray = ConcreteSecureByteArray();

		// Assert:
		EXPECT_EQ(ConcreteSecureByteArray::Size, byteArray.size());
		AssertEqual(Zero, byteArray);
	}

	TEST(TEST_CLASS, DestructorZerosOutBackingMemory) {
		// Arrange: call placement new and move constructor
		auto seed = test::GenerateRandomByteArray<ConcreteByteArray>();

		uint8_t secureMemory[ConcreteSecureByteArray::Size];
		auto pByteArray = new (secureMemory) ConcreteSecureByteArray;
		*pByteArray = MakeSecure(seed);

		// Sanity: the array's backing memory is nonzero
		EXPECT_FALSE(IsZero(secureMemory));

		// Act: destroy the array
		pByteArray->~ConcreteSecureByteArray();

		// Assert: the array's backing memory is zero
		EXPECT_TRUE(IsZero(secureMemory));
	}

	// endregion

	// region move constructor and assignment

	TEST(TEST_CLASS, MoveConstructorZerosOutBackingMemoryOfSource) {
		// Arrange: call placement new and move constructor
		auto seed = test::GenerateRandomByteArray<ConcreteByteArray>();

		uint8_t secureMemory[ConcreteSecureByteArray::Size];
		auto pByteArray = new (secureMemory) ConcreteSecureByteArray;
		*pByteArray = MakeSecure(seed);

		// Sanity: the array's backing memory is nonzero
		EXPECT_FALSE(IsZero(secureMemory));

		// Act: trigger the move constructor
		auto movedByteArray = std::move(*pByteArray);

		// Assert: the source's backing memory is zero
		EXPECT_TRUE(IsZero(secureMemory));

		// - the destination's backing memory is equal to the moved byte array
		AssertEqual(seed, movedByteArray);
	}

	TEST(TEST_CLASS, MoveAssignmentZerosOutBackingMemoryOfSource) {
		// Arrange: call placement new and move constructor
		auto seed = test::GenerateRandomByteArray<ConcreteByteArray>();

		uint8_t secureMemory[ConcreteSecureByteArray::Size];
		auto pByteArray = new (secureMemory) ConcreteSecureByteArray;
		*pByteArray = MakeSecure(seed);

		// Sanity: the array's backing memory is nonzero
		EXPECT_FALSE(IsZero(secureMemory));

		// Act: trigger the move assignment operator
		ConcreteSecureByteArray movedByteArray;
		const auto& assignResult = (movedByteArray = std::move(*pByteArray));

		// Assert: the source's backing memory is zero
		EXPECT_TRUE(IsZero(secureMemory));

		// - the destination's backing memory is equal to the moved byte array
		AssertEqual(seed, movedByteArray);

		// - the assignment operator returned the correct reference
		EXPECT_EQ(&movedByteArray, &assignResult);
	}

	// endregion

	// region factories - success

	TEST(TEST_CLASS, CanCreateFromBuffer) {
		// Arrange:
		auto seed = test::GenerateRandomByteArray<ConcreteByteArray>();
		seed[0] = 3;

		// Act:
		auto byteArray = ConcreteSecureByteArray::FromBuffer(seed);

		// Assert:
		AssertEqual(seed, byteArray);

		// - the factory function should not have zeroed out the input buffer
		EXPECT_EQ(3u, seed[0]);
	}

	TEST(TEST_CLASS, CanCreateFromBufferSecure) {
		// Arrange:
		auto seed = test::GenerateRandomByteArray<ConcreteByteArray>();
		auto seedCopy = seed;

		// Act:
		auto byteArray = ConcreteSecureByteArray::FromBufferSecure(seed);

		// Assert:
		AssertEqual(seedCopy, byteArray);

		// - the factory function should have zeroed out the input buffer
		EXPECT_EQ(ConcreteByteArray(), seed);
	}

	TEST(TEST_CLASS, CanCreateFromString) {
		// Arrange:
		auto seedString = std::string("3485D98EFD7EB07ADAFCFD1A157D89DE2796A95E780813C0");

		// Act:
		auto byteArray = ConcreteSecureByteArray::FromString(seedString);

		// Assert:
		AssertEqual(utils::ParseByteArray<ConcreteByteArray>(seedString), byteArray);

		// - the factory function should not have zeroed out the input string
		EXPECT_EQ('3', seedString[0]);
	}

	TEST(TEST_CLASS, CanCreateFromStringSecure) {
		// Arrange:
		auto seedString = std::string("3485D98EFD7EB07ADAFCFD1A157D89DE2796A95E780813C0");
		auto seedStringCopy = seedString;

		// Act:
		auto byteArray = ConcreteSecureByteArray::FromStringSecure(seedString);

		// Assert:
		AssertEqual(utils::ParseByteArray<ConcreteByteArray>(seedStringCopy), byteArray);

		// - note that the factory function should have zeroed out the input string
		EXPECT_EQ(std::string(seedString.size(), '\0'), seedString);
	}

	TEST(TEST_CLASS, CanCreateFromGenerator) {
		// Act:
		auto seed = 0;
		auto byteArray = ConcreteSecureByteArray::Generate([&seed]() {
			auto val = static_cast<uint8_t>(seed / 8 + seed % 8);
			++seed;
			return val;
		});

		// Assert:
		AssertEqual(utils::ParseByteArray<ConcreteByteArray>("000102030405060701020304050607080203040506070809"), byteArray);
	}

	// endregion

	// region factories - failure (size)

	namespace {
		std::vector<size_t> GetInvalidSizes() {
			auto size = ConcreteSecureByteArray::Size;
			return { 0, size - 10, size - 1, size + 1, size + 10 };
		}

		std::vector<char> GetInvalidCharacters() {
			return { 'g', '-' };
		}

		void AssertCannotCreateFromBufferWithSize(size_t size) {
			// Arrange:
			auto buffer = test::GenerateRandomVector(size);
			if (size > 0)
				buffer[0] = 3;

			// Act + Assert: array creation should fail but buffer should not be cleared
			EXPECT_THROW(ConcreteSecureByteArray::FromBuffer(buffer), catapult_invalid_argument) << "buffer size: " << size;

			if (size > 0)
				EXPECT_EQ(3u, buffer[0]);
		}

		void AssertCannotCreateFromSecureBufferWithSize(size_t size) {
			// Arrange:
			auto buffer = test::GenerateRandomVector(size);

			// Act + Assert: array creation should fail but buffer should be cleared
			EXPECT_THROW(ConcreteSecureByteArray::FromBufferSecure(buffer), catapult_invalid_argument) << "buffer size: " << size;
			EXPECT_EQ(std::vector<uint8_t>(size, 0), buffer);
		}

		void AssertCannotCreateFromStringWithSize(size_t size, char arrayFirstChar) {
			// Arrange:
			auto seedString = test::GenerateRandomHexString(2 * size);
			if (size > 0)
				seedString[0] = arrayFirstChar;

			// Act + Assert: array creation should fail but string should not be cleared
			EXPECT_THROW(ConcreteSecureByteArray::FromString(seedString), catapult_invalid_argument) << "string size: " << size;

			if (size > 0)
				EXPECT_EQ(arrayFirstChar, seedString[0]);
		}

		void AssertCannotCreateFromSecureStringWithSize(size_t size) {
			// Arrange:
			auto seedString = test::GenerateRandomHexString(2 * size);

			// Act + Assert: array creation should fail but string should still be cleared
			EXPECT_THROW(ConcreteSecureByteArray::FromStringSecure(seedString), catapult_invalid_argument) << "string size: " << size;
			EXPECT_EQ(std::string(seedString.size(), '\0'), seedString);
		}
	}

	TEST(TEST_CLASS, CannotCreateFromIncorrectlySizedBuffer) {
		for (auto size : GetInvalidSizes())
			AssertCannotCreateFromBufferWithSize(size);
	}

	TEST(TEST_CLASS, CannotCreateFromIncorrectlySizedSecureBuffer) {
		for (auto size : GetInvalidSizes())
			AssertCannotCreateFromSecureBufferWithSize(size);
	}

	TEST(TEST_CLASS, CannotCreateFromIncorrectlySizedString) {
		for (auto size : GetInvalidSizes()) {
			AssertCannotCreateFromStringWithSize(size, '\0');
			AssertCannotCreateFromStringWithSize(size, 'c');
		}
	}

	TEST(TEST_CLASS, CannotCreateFromIncorrectlySizedSecureString) {
		for (auto size : GetInvalidSizes())
			AssertCannotCreateFromSecureStringWithSize(size);
	}

	// endregion

	// region factories - failure (character)

	namespace {
		void AssertCannotCreateFromStringWithInvalidCharacter(char invalidChar) {
			// Arrange:
			auto seedString = test::GenerateRandomHexString(2 * ConcreteSecureByteArray::Size);
			seedString[ConcreteSecureByteArray::Size] = invalidChar;

			// Act + Assert: array creation should fail but string should not be cleared
			EXPECT_THROW(ConcreteSecureByteArray::FromString(seedString), catapult_invalid_argument) << "invalid char: " << invalidChar;
			EXPECT_EQ(invalidChar, seedString[ConcreteSecureByteArray::Size]);
		}

		void AssertCannotCreateFromSecureStringWithInvalidCharacter(char invalidChar) {
			// Arrange:
			auto seedString = test::GenerateRandomHexString(2 * ConcreteSecureByteArray::Size);
			seedString[ConcreteSecureByteArray::Size] = invalidChar;

			// Act + Assert: array creation should fail but string should still be cleared
			EXPECT_THROW(
					ConcreteSecureByteArray::FromStringSecure(seedString),
					catapult_invalid_argument) << "invalid char: " << invalidChar;
			EXPECT_EQ(std::string(seedString.size(), '\0'), seedString);
		}
	}

	TEST(TEST_CLASS, CannotCreateFromStringWithInvalidCharacter) {
		for (auto ch : GetInvalidCharacters())
			AssertCannotCreateFromStringWithInvalidCharacter(ch);
	}

	TEST(TEST_CLASS, CannotCreateFromSecureStringWithInvalidCharacter) {
		for (auto ch : GetInvalidCharacters())
			AssertCannotCreateFromSecureStringWithInvalidCharacter(ch);
	}

	// endregion

	// region comparison operators

	namespace {
		std::unordered_map<std::string, ConcreteSecureByteArray> GenerateEqualityInstanceMap() {
			auto seed = test::GenerateRandomByteArray<ConcreteByteArray>();

			std::unordered_map<std::string, ConcreteSecureByteArray> map;
			map.emplace("default", MakeSecure(seed));
			map.emplace("same-raw", MakeSecure(seed));
			map.emplace("diff-raw", MakeSecure(test::GenerateRandomByteArray<ConcreteByteArray>()));
			return map;
		}

		std::unordered_set<std::string> GetEqualTags() {
			return { "default", "same-raw" };
		}
	}

	TEST(TEST_CLASS, OperatorEqualReturnsTrueForEqualObjects) {
		test::AssertOperatorEqualReturnsTrueForEqualObjects("default", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(TEST_CLASS, OperatorNotEqualReturnsTrueForUnequalObjects) {
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects("default", GenerateEqualityInstanceMap(), GetEqualTags());
	}

	// endregion
}}
