#include "src/config/TransferConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace config {

	namespace {
		struct TransferConfigurationTraits {
			using ConfigurationType = TransferConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {{
						"",
						{
							{ "maxMessageSize", "859" }
						}
				}};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const TransferConfiguration& config) {
				// Assert:
				EXPECT_EQ(0u, config.MaxMessageSize);
			}

			static void AssertCustom(const TransferConfiguration& config) {
				// Assert:
				EXPECT_EQ(859u, config.MaxMessageSize);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(TransferConfigurationTests, Transfer)
}}
