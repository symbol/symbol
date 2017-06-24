#include "catapult/types.h"
#include "tests/TestHarness.h"

namespace catapult {

	TEST(CatapultTypesTests, AddressHasExpectedSize) {
		// Assert:
		EXPECT_EQ(Address_Decoded_Size, sizeof(Address));
	}

	TEST(CatapultTypesTests, KeyHasExpectedSize) {
		// Assert:
		EXPECT_EQ(Key_Size, sizeof(Key));
	}

	TEST(CatapultTypesTests, SignatureHasExpectedSize) {
		// Assert:
		EXPECT_EQ(Signature_Size, sizeof(Signature));
	}
}
