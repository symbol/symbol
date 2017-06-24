#include "catapult/types.h"
#include "tests/TestHarness.h"

namespace catapult {

#define TEST_CLASS CountOfTests

	namespace {
#pragma pack(push, 1)
		struct PackedCompound {
			int x;
			char y;
		};
#pragma pack(pop)

		struct NonPackedCompound {
			int x;
			char y;
		};

		struct PrimitiveTraits {
			using BaseType = int;
		};

		struct PackedTraits {
			using BaseType = PackedCompound;
		};

		struct NonPackedTraits {
			using BaseType = NonPackedCompound;
		};
	}

#define TRAIT_BASED_TEST(TEST_NAME) \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)(); \
	TEST(TEST_CLASS, TEST_NAME##_Primitive) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PrimitiveTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_Packed) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<PackedTraits>(); } \
	TEST(TEST_CLASS, TEST_NAME##_NonPacked) { TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)<NonPackedTraits>(); } \
	template<typename TTraits> void TRAITS_TEST_NAME(TEST_CLASS, TEST_NAME)()

	TRAIT_BASED_TEST(CanGetSizeOfAnArray) {
		// Arrange:
		typename TTraits::BaseType tested[5];

		// Act + Assert:
		EXPECT_EQ(5u, CountOf(tested));
	}

	TRAIT_BASED_TEST(CanGetSizeOfATwoDimensionalArray) {
		// Arrange:
		typename TTraits::BaseType tested[13][5];

		// Act + Assert:
		EXPECT_EQ(13u, CountOf(tested));
		EXPECT_EQ(5u, CountOf(tested[0]));
	}
}
