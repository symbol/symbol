#include "catapult/utils/HexFormatter.h"
#include "tests/TestHarness.h"
#include <array>

namespace catapult { namespace utils {

#define TEST_CLASS HexFormatterTests

	namespace {
		template<typename T>
		void AssertHexString(const std::string& expected, T value) {
			// Act:
			std::ostringstream out;
			out << HexFormat(value);
			std::string actual = out.str();

			// Assert:
			EXPECT_EQ(expected, actual);
		}
	}

	TEST(TEST_CLASS, CanFormatByteIntegralType) {
		// Assert:
		AssertHexString("24", static_cast<char>(0x24));
		AssertHexString("0C", static_cast<char>(0x0C));
	}

	TEST(TEST_CLASS, CanFormatShortIntegralType) {
		// Assert:
		AssertHexString("24C8", static_cast<short>(0x24C8));
		AssertHexString("0024", static_cast<short>(0x0024));
	}

	TEST(TEST_CLASS, CanFormatIntegralType) {
		// Assert:
		AssertHexString("24C81357", 0x24C81357);
		AssertHexString("0024C813", 0x0024C813);
	}

	TEST(TEST_CLASS, CanFormatLongIntegralType) {
		// Assert:
		AssertHexString("24C8135787645623", 0x24C8135787645623);
		AssertHexString("0024C81357876456", 0x0024C81357876456);
	}

	TEST(TEST_CLASS, CanFormatByteBaseValueType) {
		// Arrange:
		struct Byte_tag {};
		using Byte = BaseValue<uint8_t, Byte_tag>;

		// Assert:
		AssertHexString("24", Byte(0x24));
		AssertHexString("0C", Byte(0x0C));
	}

	TEST(TEST_CLASS, CanFormatLongBaseValueType) {
		// Assert:
		AssertHexString("24C8135787645623", Height(0x24C8135787645623));
		AssertHexString("0024C81357876456", Height(0x0024C81357876456));
	}

	namespace {
		struct TestValueRange {
			static constexpr uint16_t Min_Value = 0x0100;
			static constexpr uint16_t Max_Value = 0x8FFF;
			static constexpr uint16_t Default_Value = 0x0777;
		};
	}

	TEST(TEST_CLASS, CanFormatClampedBaseValueType) {
		// Arrange:
		using TestValue = ClampedBaseValue<uint16_t, TestValueRange>;

		// Assert:
		AssertHexString("0777", TestValue());
		AssertHexString("7123", TestValue(0x7123));
		AssertHexString("0808", TestValue(0x0808));
	}

	TEST(TEST_CLASS, CanFormatByteEnumType) {
		// Arrange:
		enum class TestByteEnum : int8_t {};

		// Assert:
		AssertHexString("24", static_cast<TestByteEnum>(0x24));
		AssertHexString("0C", static_cast<TestByteEnum>(0x0C));
	}

	TEST(TEST_CLASS, CanFormatLongEnumType) {
		// Arrange:
		enum class TestLongEnum : uint64_t {};

		// Assert:
		AssertHexString("24C8135787645623", static_cast<TestLongEnum>(0x24C8135787645623));
		AssertHexString("0024C81357876456", static_cast<TestLongEnum>(0x0024C81357876456));
	}

	TEST(TEST_CLASS, CanFormatArbitraryStruct) {
		// Arrange:
#pragma pack(push, 1)

		struct Foo {
			uint8_t Alpha;
			uint32_t Beta;
			uint16_t Gamma;
		};

#pragma pack(pop)

		Foo foo;
		foo.Alpha = 0x43;
		foo.Beta = 0x81357876;
		foo.Gamma = 0xA3B5;

		// Assert:
		AssertHexString("4376783581B5A3", foo);
	}

	TEST(TEST_CLASS, IntegralFormattingChangesDoNotLeak) {
		// Arrange:
		std::ostringstream out;
		out.flags(std::ios::dec);
		out.fill('~');

		// Act:
		out << std::setw(4) << 121 << " " << HexFormat(0x1237B9) << " " << std::setw(4) << 625;
		std::string actual = out.str();

		// Assert:
		EXPECT_EQ("~121 001237B9 ~625", actual);
	}

	namespace {
		template<typename THexFormat>
		void AssertContainerHexString(const std::string& expected, THexFormat&& hexFormat) {
			// Act:
			std::ostringstream out;
			out << hexFormat;
			std::string actual = out.str();

			// Assert:
			EXPECT_EQ(expected, actual);
		}

		struct IteratorTraits {
			template<typename TContainer>
			static auto Format(const TContainer& container) {
				return HexFormat(container.cbegin(), container.cend());
			}

			template<typename TContainer>
			static void AssertHexString(const std::string& expected, const TContainer& container) {
				AssertContainerHexString(expected, Format(container));
			}

			template<typename TContainer>
			static void AssertHexString(const std::string& expected, const TContainer& container, char separator) {
				AssertContainerHexString(expected, HexFormat(container.cbegin(), container.cend(), separator));
			}
		};

		struct ContainerTraits {
			template<typename TContainer>
			static auto Format(const TContainer& container) {
				return HexFormat(container);
			}

			template<typename TContainer>
			static void AssertHexString(const std::string& expected, const TContainer& container) {
				AssertContainerHexString(expected, Format(container));
			}

			template<typename TContainer>
			static void AssertHexString(const std::string& expected, const TContainer& container, char separator) {
				AssertContainerHexString(expected, HexFormat(container, separator));
			}
		};
	}

#define CONTAINER_TRAITS_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Iterator) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<IteratorTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Container) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ContainerTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	CONTAINER_TRAITS_BASED_TEST(CanFormatArrayByteIntegralType) {
		// Assert:
		TTraits::AssertHexString("", std::array<unsigned char, 0>());
		TTraits::AssertHexString("24", std::array<unsigned char, 1>{ { 0x24 } });
		TTraits::AssertHexString("02", std::array<unsigned char, 1>{ { 0x02 } });
		TTraits::AssertHexString("28027A", std::array<unsigned char, 3>{ { 0x28, 0x02, 0x7A } });
	}

	CONTAINER_TRAITS_BASED_TEST(CanFormatVectorShortIntegralType) {
		// Assert:
		TTraits::AssertHexString("", std::vector<short>());
		TTraits::AssertHexString("2468", std::vector<short>{ 0x2468 });
		TTraits::AssertHexString("0024", std::vector<short>{ 0x0024 });
		TTraits::AssertHexString("24680024765A", std::vector<short>{ 0x2468, 0x0024, 0x765A });
	}

	CONTAINER_TRAITS_BASED_TEST(CanFormatVectorBaseValueType) {
		// Arrange:
		struct Byte_tag {};
		using Byte = BaseValue<uint8_t, Byte_tag>;
		using ByteVector = std::vector<Byte>;

		// Assert:
		TTraits::AssertHexString("", ByteVector());
		TTraits::AssertHexString("2468", ByteVector{ Byte(0x24), Byte(0x68) });
		TTraits::AssertHexString("0024", ByteVector{ Byte(0x00), Byte(0x24) });
		TTraits::AssertHexString("24680024760A", ByteVector{ Byte(0x24), Byte(0x68), Byte(0x00), Byte(0x24), Byte(0x76), Byte(0x0A) });
	}

	CONTAINER_TRAITS_BASED_TEST(CanFormatContainerIntegralTypeWithSeparator) {
		// Assert:
		TTraits::AssertHexString("", std::vector<short>(), ',');
		TTraits::AssertHexString("2468", std::vector<short>{ 0x2468 }, ',');
		TTraits::AssertHexString("0024", std::vector<short>{ 0x0024 }, ',');
		TTraits::AssertHexString("2468,0024,765A", std::vector<short>{ 0x2468, 0x0024, 0x765A }, ',');
	}

	CONTAINER_TRAITS_BASED_TEST(ContainerIntegralFormattingChangesDoNotLeak) {
		// Arrange:
		std::ostringstream out;
		out.flags(std::ios::dec);
		out.fill('~');

		// Act:
		auto container = std::vector<short>{ 0x2468, 0x0024, 0x765A };
		out << std::setw(4) << 121 << " " << TTraits::Format(container) << " " << std::setw(4) << 625;
		std::string actual = out.str();

		// Assert:
		EXPECT_EQ("~121 24680024765A ~625", actual);
	}
}}
