#include "catapult/chain/ChainComparisonCode.h"
#include "tests/TestHarness.h"

namespace catapult { namespace chain {

#define TEST_CLASS ChainComparisonCodeTests

	TEST(TEST_CLASS, IsRemoteEvilOnlyReturnsTrueForEvilCodes) {
#define ENUM_VALUE(LABEL, VALUE) \
	EXPECT_EQ(0 != ((VALUE) & 0x80000000), IsRemoteEvil(ChainComparisonCode::LABEL));

		CHAIN_COMPARISON_CODE_LIST

#undef ENUM_VALUE
	}
}}
