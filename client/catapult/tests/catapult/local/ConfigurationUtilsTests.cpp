#include "catapult/local/ConfigurationUtils.h"
#include "catapult/config/NodeConfiguration.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS ConfigurationUtilsTests

	TEST(TEST_CLASS, CanExtractUtCacheOptionsFromNodeConfiguration) {
		// Arrange:
		auto config = config::NodeConfiguration::Uninitialized();
		config.UnconfirmedTransactionsCacheMaxResponseSize = utils::FileSize::FromKilobytes(4);
		config.UnconfirmedTransactionsCacheMaxSize = 234;

		// Act:
		auto options = GetUtCacheOptions(config);

		// Assert:
		EXPECT_EQ(4096u, options.MaxResponseSize);
		EXPECT_EQ(234u, options.MaxCacheSize);
	}
}}
