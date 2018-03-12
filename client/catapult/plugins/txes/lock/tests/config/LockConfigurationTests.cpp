#include "src/config/LockConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace config {

	namespace {
		struct LockConfigurationTraits {
			using ConfigurationType = LockConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"",
						{
							{ "lockedFundsPerAggregate", "123'000'000" },
							{ "maxHashLockDuration", "12'345d" },
							{ "maxSecretLockDuration", "23'456d" },
							{ "minProofSize", "42" },
							{ "maxProofSize", "1234" }
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const LockConfiguration& config) {
				// Assert:
				EXPECT_EQ(Amount(), config.LockedFundsPerAggregate);
				EXPECT_EQ(utils::BlockSpan(), config.MaxHashLockDuration);
				EXPECT_EQ(utils::BlockSpan(), config.MaxSecretLockDuration);
				EXPECT_EQ(0u, config.MinProofSize);
				EXPECT_EQ(0u, config.MaxProofSize);
			}

			static void AssertCustom(const LockConfiguration& config) {
				// Assert:
				EXPECT_EQ(Amount(123'000'000), config.LockedFundsPerAggregate);
				EXPECT_EQ(utils::BlockSpan::FromDays(12'345u), config.MaxHashLockDuration);
				EXPECT_EQ(utils::BlockSpan::FromDays(23'456u), config.MaxSecretLockDuration);
				EXPECT_EQ(42u, config.MinProofSize);
				EXPECT_EQ(1234u, config.MaxProofSize);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(LockConfigurationTests, Lock)
}}
