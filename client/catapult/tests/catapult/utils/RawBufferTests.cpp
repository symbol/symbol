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

#include "catapult/utils/RawBuffer.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS RawBufferTests

	namespace {
		struct BufferTraits {
			static constexpr auto GenerateRandomData = test::GenerateRandomVector;
		};

		struct RawBufferTraits : BufferTraits {
			using Type = RawBuffer;
			using ValueType = const uint8_t;
		};

		struct MutableBufferTraits : BufferTraits {
			using Type = MutableRawBuffer;
			using ValueType = uint8_t;
		};

		struct RawStringTraits {
			using Type = RawString;
			using ValueType = const char;

			static constexpr auto GenerateRandomData = test::GenerateRandomString;
		};

		struct MutableRawStringTraits {
			using Type = MutableRawString;
			using ValueType = char;

			static std::vector<char> GenerateRandomData(size_t size) {
				auto str = test::GenerateRandomString(size);
				std::vector<char> vec(size);
				std::copy(str.cbegin(), str.cend(), vec.begin());
				return vec;
			}
		};
	}

	// region all: (immutable + mutable) x (buffer + string)

#define ALL_BUFFER_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_RawBuffer) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RawBufferTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MutableBuffer) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MutableBufferTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_RawString) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RawStringTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MutableRawString) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MutableRawStringTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	ALL_BUFFER_TRAITS_BASED_TEST(CanCreateEmptyRawBuffer) {
		// Act:
		typename TTraits::Type buffer;

		// Assert:
		ASSERT_EQ(0u, buffer.Size);
		EXPECT_FALSE(!!buffer.pData);
	}

	ALL_BUFFER_TRAITS_BASED_TEST(CanCreateRawBufferAroundEntireContainer) {
		// Arrange:
		auto input = TTraits::GenerateRandomData(25);

		// Act:
		typename TTraits::Type buffer(input);

		// Assert: pointer comparison
		ASSERT_EQ(25u, buffer.Size);
		EXPECT_EQ(input.data(), buffer.pData);
	}

	ALL_BUFFER_TRAITS_BASED_TEST(CanCreateRawBufferAroundEntireTemporaryContainer) {
		// Arrange:
		auto input = TTraits::GenerateRandomData(25);

		// Act:
		[&input](const typename TTraits::Type& buffer) {
			// Assert:
			ASSERT_EQ(25u, buffer.Size);
			EXPECT_EQ_MEMORY(&input[0], buffer.pData, buffer.Size);
		}(decltype(input)(input)); // call the lambda with a (temporary) copy of input
	}

	ALL_BUFFER_TRAITS_BASED_TEST(CanCreateRawBufferAroundPartialContainer) {
		// Arrange:
		auto input = TTraits::GenerateRandomData(25);

		// Act:
		typename TTraits::Type buffer(input.data() + 5, 6);

		// Assert: pointer comparison
		ASSERT_EQ(6u, buffer.Size);
		EXPECT_EQ(input.data() + 5, buffer.pData);
	}

	ALL_BUFFER_TRAITS_BASED_TEST(CanCopyConstructRawBuffer) {
		// Arrange:
		auto input = TTraits::GenerateRandomData(25);

		// Act:
		typename TTraits::Type originalBuffer(input.data() + 5, 6);
		typename TTraits::Type buffer(originalBuffer);

		// Assert: pointer comparison
		ASSERT_EQ(6u, buffer.Size);
		EXPECT_EQ(input.data() + 5, buffer.pData);
	}

	ALL_BUFFER_TRAITS_BASED_TEST(CanCopyRawBuffer) {
		// Arrange:
		auto input = TTraits::GenerateRandomData(25);

		// Act:
		typename TTraits::Type originalBuffer(input.data() + 5, 6);
		typename TTraits::Type buffer;
		buffer = originalBuffer;

		// Assert: pointer comparison
		ASSERT_EQ(6u, buffer.Size);
		EXPECT_EQ(input.data() + 5, buffer.pData);
	}

	namespace {
		template<typename T>
		size_t Foo(const BasicRawBuffer<T>&) {
			return 1;
		}

		template<typename T>
		size_t Foo(std::initializer_list<const BasicRawBuffer<T>>) {
			return 2;
		}
	}

	ALL_BUFFER_TRAITS_BASED_TEST(CanResolveProperlyWhenInitializerListOverloadIsPresent) {
		// Arrange:
		auto input = TTraits::GenerateRandomData(25);

		// Act:
		using ValueType = typename TTraits::ValueType;
		auto result1 = Foo<ValueType>({ input.data(), input.size() }); // should call BasicRawBuffer overload
		auto result2 = Foo<ValueType>({ { input }, { input.data(), input.size() } }); // should call initializer_list overload

		// Assert:
		EXPECT_EQ(1u, result1);
		EXPECT_EQ(2u, result2);
	}

	// endregion

	// region mutable: (mutable) x (buffer + string)

#define MUTABLE_BUFFER_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_MutableBuffer) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MutableBufferTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MutableRawString) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MutableRawStringTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	MUTABLE_BUFFER_TRAITS_BASED_TEST(CanCreateMutableRawBufferAroundEntireContainer) {
		// Arrange:
		auto input = TTraits::GenerateRandomData(25);
		auto originalByte = input[0];

		// Act:
		typename TTraits::Type buffer(input);
		buffer.pData[0] = static_cast<decltype(originalByte)>(buffer.pData[0] ^ 0xFF);

		// Assert: pointer comparison
		ASSERT_EQ(25u, buffer.Size);
		EXPECT_EQ(input.data(), buffer.pData);
		EXPECT_EQ(static_cast<decltype(originalByte)>(originalByte ^ 0xFF), input[0]);
	}

	MUTABLE_BUFFER_TRAITS_BASED_TEST(CanCreateMutableRawBufferAroundPartialContainer) {
		// Arrange:
		auto input = TTraits::GenerateRandomData(25);
		auto originalByte = input[7];

		// Act:
		typename TTraits::Type buffer(input.data() + 5, 6);
		buffer.pData[2] = static_cast<decltype(originalByte)>(buffer.pData[2] ^ 0xFF);

		// Assert: pointer comparison
		ASSERT_EQ(6u, buffer.Size);
		EXPECT_EQ(input.data() + 5, buffer.pData);
		EXPECT_EQ(static_cast<decltype(originalByte)>(originalByte ^ 0xFF), input[7]);
	}

	// endregion

	// region string: (immutable + mutable) x (string)

#define STRING_BUFFER_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_RawString) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<RawStringTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_MutableRawString) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<MutableRawStringTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	STRING_BUFFER_TRAITS_BASED_TEST(CanOutputEmptyRawString) {
		// Arrange:
		typename TTraits::Type str;

		// Act:
		std::stringstream out;
		out << str;

		// Assert:
		EXPECT_EQ("", out.str());
	}

	STRING_BUFFER_TRAITS_BASED_TEST(CanOutputRawString) {
		// Arrange:
		std::string input("abcdef");
		typename TTraits::Type str(&input[0], 3);

		// Act:
		std::stringstream out;
		out << str;

		// Assert:
		EXPECT_EQ("abc", out.str());
	}

	STRING_BUFFER_TRAITS_BASED_TEST(CanCreateRawStringAroundStlString) {
		// Arrange:
		auto input = test::GenerateRandomString(25);

		// Act:
		typename TTraits::Type str(input);

		// Assert:
		EXPECT_EQ(25u, str.Size);
		EXPECT_EQ(input.data(), str.pData);
		EXPECT_EQ(input, std::string(str.pData, str.Size));
	}

	// endregion

	// region immutable string: (immutable) x (string)

	TEST(TEST_CLASS, CanCreateRawStringAroundTemporaryStlString) {
		// Act:
		[](const RawString& str) {
			// Assert:
			EXPECT_EQ(12u, str.Size);
			EXPECT_STREQ("hello world!", str.pData);
		}(std::string("hello ") + std::string("world!"));
	}

	TEST(TEST_CLASS, CanCreateRawStringAroundStringLiteral) {
		// Act:
		[](const RawString& str) {
			// Assert:
			EXPECT_EQ(12u, str.Size);
			EXPECT_STREQ("hello world!", str.pData);
		}("hello world!");
	}

	// endregion
}}
