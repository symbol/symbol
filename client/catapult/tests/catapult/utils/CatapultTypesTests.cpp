#include "catapult/types.h"
#include "tests/TestHarness.h"

namespace catapult {

#define TEST_CLASS CatapultTypesTests

	TEST(TEST_CLASS, AddressHasExpectedSize) {
		// Assert:
		EXPECT_EQ(Address_Decoded_Size, sizeof(Address));
	}

	TEST(TEST_CLASS, KeyHasExpectedSize) {
		// Assert:
		EXPECT_EQ(Key_Size, sizeof(Key));
	}

	TEST(TEST_CLASS, SignatureHasExpectedSize) {
		// Assert:
		EXPECT_EQ(Signature_Size, sizeof(Signature));
	}
}
