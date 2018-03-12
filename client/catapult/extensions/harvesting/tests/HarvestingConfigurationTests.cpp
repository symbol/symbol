#include "harvesting/src/HarvestingConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace harvesting {

	namespace {
		struct HarvestingConfigurationTraits {
			using ConfigurationType = HarvestingConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"harvesting",
						{
							{ "harvestKey", "harvest-key" },
							{ "isAutoHarvestingEnabled", "true" },
							{ "maxUnlockedAccounts", "2" }
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const HarvestingConfiguration& config) {
				// Assert:
				EXPECT_EQ("", config.HarvestKey);
				EXPECT_FALSE(config.IsAutoHarvestingEnabled);
				EXPECT_EQ(0u, config.MaxUnlockedAccounts);
			}

			static void AssertCustom(const HarvestingConfiguration& config) {
				// Assert:
				EXPECT_EQ("harvest-key", config.HarvestKey);
				EXPECT_TRUE(config.IsAutoHarvestingEnabled);
				EXPECT_EQ(2u, config.MaxUnlockedAccounts);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(HarvestingConfigurationTests, Harvesting)
}}
