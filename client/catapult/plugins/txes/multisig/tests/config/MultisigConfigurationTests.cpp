#include "src/config/MultisigConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace config {

	namespace {
		struct MultisigConfigurationTraits {
			using ConfigurationType = MultisigConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {{
						"",
						{
							{ "maxMultisigDepth", "159" },
							{ "maxCosignersPerAccount", "23" },
							{ "maxCosignedAccountsPerAccount", "77" },
						}
				}};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const MultisigConfiguration& config) {
				// Assert:
				EXPECT_EQ(0u, config.MaxMultisigDepth);
				EXPECT_EQ(0u, config.MaxCosignersPerAccount);
				EXPECT_EQ(0u, config.MaxCosignedAccountsPerAccount);
			}

			static void AssertCustom(const MultisigConfiguration& config) {
				// Assert:
				EXPECT_EQ(159u, config.MaxMultisigDepth);
				EXPECT_EQ(23u, config.MaxCosignersPerAccount);
				EXPECT_EQ(77u, config.MaxCosignedAccountsPerAccount);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(MultisigConfigurationTests, Multisig)
}}
