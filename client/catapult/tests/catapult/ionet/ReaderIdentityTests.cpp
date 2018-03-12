#include "catapult/ionet/ReaderIdentity.h"
#include "catapult/crypto/KeyUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace ionet {

	TEST(TEST_CLASS, CanOutputReaderIdentity) {
		// Arrange:
		auto key = crypto::ParseKey("1B664F8BDA2DBF33CB6BE21C8EB3ECA9D9D5BF144C08E9577ED0D1E5E5608751");
		auto identity = ReaderIdentity{ key, "bob.com" };

		// Act:
		auto str = test::ToString(identity);

		// Assert:
		EXPECT_EQ("reader (1B664F8BDA2DBF33CB6BE21C8EB3ECA9D9D5BF144C08E9577ED0D1E5E5608751 @ bob.com)", str);
	}
}}
