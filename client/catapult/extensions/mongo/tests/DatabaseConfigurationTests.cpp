#include "mongo/src/DatabaseConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace mongo {

#define TEST_CLASS DatabaseConfigurationTests

	namespace {
		struct DatabaseConfigurationTraits {
			using ConfigurationType = DatabaseConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"database",
						{
							{ "databaseUri", "mongodb://hostname:port" },
							{ "databaseName", "foo" },
							{ "maxWriterThreads", "3" }
						}
					},
					{
						"plugins",
						{
							{ "Alpha", "true" },
							{ "BETA", "false" },
							{ "gamma", "true" }
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string& section) {
				return "plugins" == section;
			}

			static void AssertZero(const DatabaseConfiguration& config) {
				// Assert:
				EXPECT_EQ("", config.DatabaseUri);
				EXPECT_EQ("", config.DatabaseName);
				EXPECT_EQ(0u, config.MaxWriterThreads);
				EXPECT_EQ(std::unordered_set<std::string>(), config.Plugins);
			}

			static void AssertCustom(const DatabaseConfiguration& config) {
				// Assert:
				EXPECT_EQ("mongodb://hostname:port", config.DatabaseUri);
				EXPECT_EQ("foo", config.DatabaseName);
				EXPECT_EQ(3u, config.MaxWriterThreads);
				EXPECT_EQ(std::unordered_set<std::string>({ "Alpha", "gamma" }), config.Plugins);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(TEST_CLASS, Database)

	// region file io

	TEST(TEST_CLASS, LoadFromPathFailsIfFileDoesNotExist) {
		// Act + Assert: attempt to load the config
		EXPECT_THROW(DatabaseConfiguration::LoadFromPath("../no-resources"), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanLoadConfigFromResourcesDirectory) {
		// Act: attempt to load from the "real" resources directory
		auto config = DatabaseConfiguration::LoadFromPath("../resources");

		// Assert:
		EXPECT_EQ("mongodb://127.0.0.1:27017", config.DatabaseUri);
		EXPECT_EQ("catapult", config.DatabaseName);
		EXPECT_EQ(8u, config.MaxWriterThreads);
		EXPECT_FALSE(config.Plugins.empty());
	}

	// endregion
}}
