#include "catapult/utils/RawBuffer.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS RawBufferTests

	namespace {
		struct BufferTraits {
			static constexpr auto GenerateRandomData = test::GenerateRandomVector;

			static std::string ToString(const uint8_t* pData, size_t dataSize) {
				return test::ToHexString(pData, dataSize);
			}
		};

		struct RawBufferTraits : BufferTraits {
			using Type = RawBuffer;
			using ValueType = const uint8_t;
		};

		struct MutableBufferTraits : BufferTraits {
			using Type = MutableRawBuffer;
			using ValueType = uint8_t;
		};

		struct StringTraits {
			static std::string ToString(const char* pData, size_t dataSize) {
				return std::string(pData, dataSize);
			}
		};

		struct RawStringTraits : StringTraits{
			using Type = RawString;
			using ValueType = const char;

			static constexpr auto GenerateRandomData = test::GenerateRandomString;
		};

		struct MutableRawStringTraits : StringTraits {
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
		EXPECT_EQ(0u, buffer.Size);
		EXPECT_FALSE(!!buffer.pData);
	}

	ALL_BUFFER_TRAITS_BASED_TEST(CanCreateRawBufferAroundEntireContainer) {
		// Arrange:
		auto data = TTraits::GenerateRandomData(25);

		// Act:
		typename TTraits::Type buffer(data);

		// Assert:
		EXPECT_EQ(25u, buffer.Size);
		EXPECT_EQ(data.data(), buffer.pData);
		EXPECT_EQ(TTraits::ToString(data.data(), data.size()), TTraits::ToString(buffer.pData, buffer.Size));
	}

	ALL_BUFFER_TRAITS_BASED_TEST(CanCreateRawBufferAroundEntireTemporaryContainer) {
		// Arrange:
		auto data = TTraits::GenerateRandomData(25);

		// Act:
		[&data](const typename TTraits::Type& buffer) {
			// Assert:
			EXPECT_EQ(25u, buffer.Size);
			EXPECT_EQ(TTraits::ToString(data.data(), data.size()), TTraits::ToString(buffer.pData, buffer.Size));
		}(decltype(data)(data)); // call the lambda with a (temporary) copy of data
	}

	ALL_BUFFER_TRAITS_BASED_TEST(CanCreateRawBufferAroundPartialContainer) {
		// Arrange:
		auto data = TTraits::GenerateRandomData(25);

		// Act:
		typename TTraits::Type buffer(data.data() + 5, 6);

		// Assert:
		EXPECT_EQ(6u, buffer.Size);
		EXPECT_EQ(data.data() + 5, buffer.pData);
		EXPECT_EQ(TTraits::ToString(data.data() + 5, 6), TTraits::ToString(buffer.pData, buffer.Size));
	}

	ALL_BUFFER_TRAITS_BASED_TEST(CanCopyConstructRawBuffer) {
		// Arrange:
		auto data = TTraits::GenerateRandomData(25);

		// Act:
		typename TTraits::Type originalBuffer(data.data() + 5, 6);
		typename TTraits::Type buffer(originalBuffer);

		// Assert:
		EXPECT_EQ(6u, buffer.Size);
		EXPECT_EQ(data.data() + 5, buffer.pData);
		EXPECT_EQ(TTraits::ToString(data.data() + 5, 6), TTraits::ToString(buffer.pData, buffer.Size));
	}

	ALL_BUFFER_TRAITS_BASED_TEST(CanCopyRawBuffer) {
		// Arrange:
		auto data = TTraits::GenerateRandomData(25);

		// Act:
		typename TTraits::Type originalBuffer(data.data() + 5, 6);
		typename TTraits::Type buffer;
		buffer = originalBuffer;

		// Assert:
		EXPECT_EQ(6u, buffer.Size);
		EXPECT_EQ(data.data() + 5, buffer.pData);
		EXPECT_EQ(TTraits::ToString(data.data() + 5, 6), TTraits::ToString(buffer.pData, buffer.Size));
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
		auto data = TTraits::GenerateRandomData(25);

		// Act:
		using ValueType = typename TTraits::ValueType;
		auto result1 = Foo<ValueType>({ data.data(), data.size() }); // should call BasicRawBuffer overload
		auto result2 = Foo<ValueType>({ { data }, { data.data(), data.size() } }); // should call initializer_list overload

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
		auto data = TTraits::GenerateRandomData(25);
		auto originalByte = data[0];

		// Act:
		typename TTraits::Type buffer(data);
		buffer.pData[0] ^= 0xFF;

		// Assert:
		EXPECT_EQ(25u, buffer.Size);
		EXPECT_EQ(data.data(), buffer.pData);
		EXPECT_EQ(TTraits::ToString(data.data(), data.size()), TTraits::ToString(buffer.pData, buffer.Size));
		EXPECT_EQ(static_cast<decltype(originalByte)>(originalByte ^ 0xFF), data[0]);
	}

	MUTABLE_BUFFER_TRAITS_BASED_TEST(CanCreateMutableRawBufferAroundPartialContainer) {
		// Arrange:
		auto data = TTraits::GenerateRandomData(25);
		auto originalByte = data[7];

		// Act:
		typename TTraits::Type buffer(data.data() + 5, 6);
		buffer.pData[2] ^= 0xFF;

		// Assert:
		EXPECT_EQ(6u, buffer.Size);
		EXPECT_EQ(data.data() + 5, buffer.pData);
		EXPECT_EQ(TTraits::ToString(data.data() + 5, 6), TTraits::ToString(buffer.pData, buffer.Size));
		EXPECT_EQ(static_cast<decltype(originalByte)>(originalByte ^ 0xFF), data[7]);
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

	STRING_BUFFER_TRAITS_BASED_TEST(CanOutputNonEmptyRawString) {
		// Arrange:
		std::string data("abcdef");
		typename TTraits::Type str(&data[0], 3);

		// Act:
		std::stringstream out;
		out << str;

		// Assert:
		EXPECT_EQ("abc", out.str());
	}

	STRING_BUFFER_TRAITS_BASED_TEST(CanCreateRawStringAroundStlString) {
		// Arrange:
		auto data = test::GenerateRandomString(25);

		// Act:
		typename TTraits::Type str(data);

		// Assert:
		EXPECT_EQ(25u, str.Size);
		EXPECT_EQ(data.data(), str.pData);
		EXPECT_EQ(data, std::string(str.pData, str.Size));
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
