#include "sync/src/TasksConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace sync {

#define TEST_CLASS TaskConfigurationTests

	namespace {
		struct TaskConfigurationTraits {
			using ConfigurationType = TaskConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"",
						{
							{ "startDelay", "1m" },
							{ "repeatDelay", "37s" },
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const TaskConfiguration& config) {
				// Assert:
				EXPECT_EQ(utils::TimeSpan(), config.StartDelay);
				EXPECT_EQ(utils::TimeSpan(), config.RepeatDelay);
			}

			static void AssertCustom(const TaskConfiguration& config) {
				// Assert:
				EXPECT_EQ(utils::TimeSpan::FromMinutes(1), config.StartDelay);
				EXPECT_EQ(utils::TimeSpan::FromSeconds(37), config.RepeatDelay);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(TEST_CLASS, Task)

	// region tasks (bag)

	TEST(TEST_CLASS, Tasks_CanCreateUninitializedConfiguration) {
		// Act:
		auto config = TasksConfiguration::Uninitialized();

		// Assert:
		EXPECT_TRUE(config.Tasks.empty());
	}

	namespace {
		void AssertContains(
				const TasksConfiguration& config,
				const std::string& expectedName,
				const utils::TimeSpan& expectedStartDelay,
				const utils::TimeSpan& expectedRepeatDelay) {
			// Assert:
			auto iter = config.Tasks.find(expectedName);
			ASSERT_TRUE(config.Tasks.cend() != iter) << expectedName;
			EXPECT_EQ(expectedStartDelay, iter->second.StartDelay) << expectedName;
			EXPECT_EQ(expectedRepeatDelay, iter->second.RepeatDelay) << expectedName;
		}
	}

	TEST(TEST_CLASS, Tasks_LoadFromBagCanLoadFromEmptyBag) {
		// Arrange:
		utils::ConfigurationBag bag({});

		// Act:
		auto config = TasksConfiguration::LoadFromBag(bag);

		// Assert:
		EXPECT_TRUE(config.Tasks.empty());
	}

	TEST(TEST_CLASS, Tasks_LoadFromBagCanLoadFromNonEmptyBag) {
		// Arrange:
		utils::ConfigurationBag bag({
			{ "alpha", { { "startDelay", "20s" }, { "repeatDelay", "45s" } } },
			{ "beta", { { "startDelay", "11m" }, { "repeatDelay", "22m" } } },
			{ "gamma", { { "startDelay", "10s" }, { "repeatDelay", "2m" } } }
		});

		// Act:
		auto config = TasksConfiguration::LoadFromBag(bag);

		// Assert:
		EXPECT_EQ(3u, config.Tasks.size());
		AssertContains(config, "alpha", utils::TimeSpan::FromSeconds(20), utils::TimeSpan::FromSeconds(45));
		AssertContains(config, "beta", utils::TimeSpan::FromMinutes(11), utils::TimeSpan::FromMinutes(22));
		AssertContains(config, "gamma", utils::TimeSpan::FromSeconds(10), utils::TimeSpan::FromMinutes(2));
	}

	TEST(TEST_CLASS, Tasks_LoadFromBagFailsIfAnyTaskHasInvalidValue) {
		// Arrange:
		utils::ConfigurationBag bag({
			{ "alpha", { { "startDelay", "20s" }, { "repeatDelay", "45s" } } },
			{ "beta", { { "startDelay", "11x" }, { "repeatDelay", "22m" } } },
			{ "gamma", { { "startDelay", "10s" }, { "repeatDelay", "2m" } } }
		});

		// Act + Assert:
		EXPECT_THROW(TasksConfiguration::LoadFromBag(bag), utils::property_malformed_error);
	}

	TEST(TEST_CLASS, Tasks_LoadFromBagFailsIfAnyTaskHasMissingValue) {
		// Arrange:
		utils::ConfigurationBag bag({
			{ "alpha", { { "startDelay", "20s" }, { "repeatDelay", "45s" } } },
			{ "beta", { { "startDelay", "11m" } } },
			{ "gamma", { { "startDelay", "10s" }, { "repeatDelay", "2m" } } }
		});

		// Act + Assert:
		EXPECT_THROW(TasksConfiguration::LoadFromBag(bag), utils::property_not_found_error);
	}

	TEST(TEST_CLASS, Tasks_LoadFromBagFailsIfAnyTaskHasExtraValue) {
		// Arrange:
		utils::ConfigurationBag bag({
			{ "alpha", { { "startDelay", "20s" }, { "repeatDelay", "45s" } } },
			{ "beta", { { "startDelay", "11m" }, { "extraDelay", "15m" }, { "repeatDelay", "22m" } } },
			{ "gamma", { { "startDelay", "10s" }, { "repeatDelay", "2m" } } }
		});

		// Act + Assert:
		EXPECT_THROW(TasksConfiguration::LoadFromBag(bag), catapult_invalid_argument);
	}

	// endregion

	// region tasks (file io)

	TEST(TEST_CLASS, Tasks_LoadFromPathFailsIfFileDoesNotExist) {
		// Act + Assert: attempt to load the config
		EXPECT_THROW(TasksConfiguration::LoadFromPath("../no-resources"), catapult_runtime_error);
	}

	TEST(TEST_CLASS, Tasks_CanLoadConfigFromResourcesDirectory) {
		// Act: attempt to load from the "real" resources directory
		auto config = TasksConfiguration::LoadFromPath("../resources");

		// Assert:
		EXPECT_EQ(14u, config.Tasks.size());

		// - spot check one task
		AssertContains(config, "harvesting task", utils::TimeSpan::FromSeconds(30), utils::TimeSpan::FromSeconds(1));
	}

	// endregion
}}
