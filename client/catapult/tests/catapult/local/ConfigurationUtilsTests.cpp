#include "catapult/local/ConfigurationUtils.h"
#include "catapult/config/NodeConfiguration.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

	TEST(ConfigurationUtilsTests, CanExtractUnconfirmedTransactionsCacheOptionsFromNodeConfiguration) {
		// Arrange:
		auto config = config::NodeConfiguration::Uninitialized();
		config.UnconfirmedTransactionsCacheMaxResponseSize = utils::FileSize::FromKilobytes(4);
		config.UnconfirmedTransactionsCacheMaxSize = 234;

		// Act:
		auto options = GetUnconfirmedTransactionsCacheOptions(config);

		// Assert:
		EXPECT_EQ(4096u, options.MaxResponseSize);
		EXPECT_EQ(234u, options.MaxCacheSize);
	}
}}
