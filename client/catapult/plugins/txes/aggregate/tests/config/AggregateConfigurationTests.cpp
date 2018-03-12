#include "src/config/AggregateConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace config {

	namespace {
		struct AggregateConfigurationTraits {
			using ConfigurationType = AggregateConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"",
						{
							{ "maxTransactionsPerAggregate", "674" },
							{ "maxCosignaturesPerAggregate", "52" },
							{ "enableStrictCosignatureCheck", "true" },
							{ "enableBondedAggregateSupport", "true" }
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const AggregateConfiguration& config) {
				// Assert:
				EXPECT_EQ(0u, config.MaxTransactionsPerAggregate);
				EXPECT_EQ(0u, config.MaxCosignaturesPerAggregate);
				EXPECT_FALSE(config.EnableStrictCosignatureCheck);
				EXPECT_FALSE(config.EnableBondedAggregateSupport);
			}

			static void AssertCustom(const AggregateConfiguration& config) {
				// Assert:
				EXPECT_EQ(674u, config.MaxTransactionsPerAggregate);
				EXPECT_EQ(52u, config.MaxCosignaturesPerAggregate);
				EXPECT_TRUE(config.EnableStrictCosignatureCheck);
				EXPECT_TRUE(config.EnableBondedAggregateSupport);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(AggregateConfigurationTests, Aggregate)
}}
