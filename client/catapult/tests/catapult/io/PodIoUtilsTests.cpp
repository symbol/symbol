#include "catapult/io/PodIoUtils.h"
#include "catapult/io/RawFile.h"
#include "tests/test/nodeps/Filesystem.h"
#include "tests/TestHarness.h"

using catapult::test::TempFileGuard;

namespace catapult { namespace io {

#define TEST_CLASS PodIoUtilsTests

	namespace {
		struct ReadReturnValueTraits {
			template<typename T>
			static T Read(RawFile& file) {
				// Act: use return value read
				return io::Read<T>(file);
			}
		};

		struct ReadOutParameterTraits {
			template<typename T>
			static T Read(RawFile& file) {
				// Act: use out parameter read
				T value;
				io::Read(file, value);
				return value;
			}
		};

#define ROUNDTRIP_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_ReadReturnValue) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ReadReturnValueTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_ReadOutParameter) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<ReadOutParameterTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

		template<typename TReadTraits, typename T>
		T RoundtripPod(const T& source) {
			// Arrange:
			TempFileGuard guard("test.dat");

			// Act:
			{
				RawFile file(guard.name(), OpenMode::Read_Write);
				Write(file, source);
			}
			T actual;
			{
				RawFile file(guard.name(), OpenMode::Read_Only);
				actual = TReadTraits::template Read<T>(file);
			}

			return actual;
		}

#pragma pack(push, 1)
		struct SampleData {
			std::array<uint8_t, 10> Buffer;
			uint32_t Value32;
			double ValueDouble;
		};
#pragma pack(pop)
	}

	ROUNDTRIP_TEST(CanRoundtripBasicType) {
		// Arrange:
		constexpr auto Expected = 0x12345678'90ABCDEFull;

		// Act:
		auto actual = RoundtripPod<TTraits>(Expected);

		// Assert:
		EXPECT_EQ(Expected, actual);
	}

	ROUNDTRIP_TEST(CanRoundtripCompoundType) {
		// Arrange:
		constexpr SampleData Expected = {
			{ { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 } },
			0x12345678u,
			1.234567890
		};

		// Act:
		auto actual = RoundtripPod<TTraits>(Expected);

		// Assert:
		EXPECT_EQ(Expected.Buffer, actual.Buffer);
		EXPECT_EQ(Expected.Value32, actual.Value32);
		EXPECT_EQ(Expected.ValueDouble, actual.ValueDouble);
	}
}}
