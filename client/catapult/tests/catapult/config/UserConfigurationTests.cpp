#include "catapult/config/UserConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace config {

	namespace {
		struct UserConfigurationTraits {
			using ConfigurationType = UserConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {{
						"account",
						{
							{ "bootKey", "boot-key" },
							{ "harvestKey", "harvest-key" },
							{ "isAutoHarvestingEnabled", "true" },
							{ "maxUnlockedAccounts", "2" }
						}
				}, {
						"storage",
						{
							{ "dataDirectory", "./db" },
							{ "pluginsDirectory", "./ext" }
						}
				}};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const UserConfiguration& config) {
				// Assert:
				EXPECT_EQ("", config.BootKey);
				EXPECT_EQ("", config.HarvestKey);
				EXPECT_FALSE(config.IsAutoHarvestingEnabled);
				EXPECT_EQ(0u, config.MaxUnlockedAccounts);

				EXPECT_EQ("", config.DataDirectory);
				EXPECT_EQ("", config.PluginsDirectory);
			}

			static void AssertCustom(const UserConfiguration& config) {
				// Assert:
				EXPECT_EQ("boot-key", config.BootKey);
				EXPECT_EQ("harvest-key", config.HarvestKey);
				EXPECT_TRUE(config.IsAutoHarvestingEnabled);
				EXPECT_EQ(2u, config.MaxUnlockedAccounts);

				EXPECT_EQ("./db", config.DataDirectory);
				EXPECT_EQ("./ext", config.PluginsDirectory);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(UserConfigurationTests, User)
}}
