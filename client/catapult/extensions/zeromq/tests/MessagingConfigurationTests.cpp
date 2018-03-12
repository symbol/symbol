#include "zeromq/src/MessagingConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace zeromq {

#define TEST_CLASS MessagingConfigurationTests

	namespace {
		struct MessagingConfigurationTraits {
			using ConfigurationType = MessagingConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"messaging",
						{
							{ "subscriberPort", "9753" }
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const MessagingConfiguration& config) {
				// Assert:
				EXPECT_EQ(0u, config.SubscriberPort);
			}

			static void AssertCustom(const MessagingConfiguration& config) {
				// Assert:
				EXPECT_EQ(9753u, config.SubscriberPort);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(TEST_CLASS, Messaging)

	// region file io

	TEST(TEST_CLASS, LoadFromPathFailsIfFileDoesNotExist) {
		// Act + Assert: attempt to load the config
		EXPECT_THROW(MessagingConfiguration::LoadFromPath("../no-resources"), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanLoadConfigFromResourcesDirectory) {
		// Act: attempt to load from the "real" resources directory
		auto config = MessagingConfiguration::LoadFromPath("../resources");

		// Assert:
		EXPECT_EQ(7902u, config.SubscriberPort);
	}

	// endregion
}}
