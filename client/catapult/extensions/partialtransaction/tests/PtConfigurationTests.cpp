#include "partialtransaction/src/PtConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace partialtransaction {

#define TEST_CLASS PtConfigurationTests

	namespace {
		struct PtConfigurationTraits {
			using ConfigurationType = PtConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"partialtransactions",
						{
							{ "cacheMaxResponseSize", "234KB" },
							{ "cacheMaxSize", "98'763" },
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const PtConfiguration& config) {
				// Assert:
				EXPECT_EQ(utils::FileSize::FromMegabytes(0), config.CacheMaxResponseSize);
				EXPECT_EQ(0u, config.CacheMaxSize);
			}

			static void AssertCustom(const PtConfiguration& config) {
				// Assert:
				EXPECT_EQ(utils::FileSize::FromKilobytes(234), config.CacheMaxResponseSize);
				EXPECT_EQ(98'763u, config.CacheMaxSize);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(TEST_CLASS, Pt)

	// region file io

	TEST(TEST_CLASS, LoadFromPathFailsIfFileDoesNotExist) {
		// Act + Assert: attempt to load the config
		EXPECT_THROW(PtConfiguration::LoadFromPath("../no-resources"), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanLoadConfigFromResourcesDirectory) {
		// Act: attempt to load from the "real" resources directory
		auto config = PtConfiguration::LoadFromPath("../resources");

		// Assert:
		EXPECT_EQ(utils::FileSize::FromMegabytes(20), config.CacheMaxResponseSize);
		EXPECT_EQ(1'000'000u, config.CacheMaxSize);
	}

	// endregion
}}
