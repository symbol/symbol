#include "catapult/config/UserConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace config {

	namespace {
		struct UserConfigurationTraits {
			using ConfigurationType = UserConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"account",
						{
							{ "bootKey", "boot-key" }
						}
					},
					{
						"storage",
						{
							{ "dataDirectory", "./db" },
							{ "pluginsDirectory", "./ext" }
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const UserConfiguration& config) {
				// Assert:
				EXPECT_EQ("", config.BootKey);

				EXPECT_EQ("", config.DataDirectory);
				EXPECT_EQ("", config.PluginsDirectory);
			}

			static void AssertCustom(const UserConfiguration& config) {
				// Assert:
				EXPECT_EQ("boot-key", config.BootKey);

				EXPECT_EQ("./db", config.DataDirectory);
				EXPECT_EQ("./ext", config.PluginsDirectory);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(UserConfigurationTests, User)
}}
