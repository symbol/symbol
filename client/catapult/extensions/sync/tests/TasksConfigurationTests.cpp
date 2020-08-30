/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "sync/src/TasksConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace sync {

#define TEST_CLASS TasksConfigurationTests

	using TimeSpan = utils::TimeSpan;

	// region UniformTaskConfiguration

	namespace {
		struct UniformTaskConfigurationTraits {
			using ConfigurationType = UniformTaskConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"",
						{
							{ "startDelay", "1m" },
							{ "repeatDelay", "37s" }
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const ConfigurationType& config) {
				// Assert:
				EXPECT_EQ(TimeSpan(), config.StartDelay);
				EXPECT_EQ(TimeSpan(), config.RepeatDelay);
			}

			static void AssertCustom(const ConfigurationType& config) {
				// Assert:
				EXPECT_EQ(TimeSpan::FromMinutes(1), config.StartDelay);
				EXPECT_EQ(TimeSpan::FromSeconds(37), config.RepeatDelay);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(TEST_CLASS, UniformTask)

	// endregion

	// region DeceleratingTaskConfiguration

	namespace {
		struct DeceleratingTaskConfigurationTraits {
			using ConfigurationType = DeceleratingTaskConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"",
						{
							{ "startDelay", "2m" },
							{ "minDelay", "42s" },
							{ "maxDelay", "14s" },
							{ "numPhaseOneRounds", "17" },
							{ "numTransitionRounds", "103" }
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const ConfigurationType& config) {
				// Assert:
				EXPECT_EQ(TimeSpan(), config.StartDelay);
				EXPECT_EQ(TimeSpan(), config.MinDelay);
				EXPECT_EQ(TimeSpan(), config.MaxDelay);
				EXPECT_EQ(0u, config.NumPhaseOneRounds);
				EXPECT_EQ(0u, config.NumTransitionRounds);
			}

			static void AssertCustom(const ConfigurationType& config) {
				// Assert:
				EXPECT_EQ(TimeSpan::FromMinutes(2), config.StartDelay);
				EXPECT_EQ(TimeSpan::FromSeconds(42), config.MinDelay);
				EXPECT_EQ(TimeSpan::FromSeconds(14), config.MaxDelay);
				EXPECT_EQ(17u, config.NumPhaseOneRounds);
				EXPECT_EQ(103u, config.NumTransitionRounds);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(TEST_CLASS, DeceleratingTask)

	// endregion

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
				const TimeSpan& expectedStartDelay,
				const TimeSpan& expectedRepeatDelay) {
			// Assert:
			auto iter = config.Tasks.find(expectedName);
			ASSERT_TRUE(config.Tasks.cend() != iter) << expectedName;
			ASSERT_EQ(TasksConfiguration::TaskType::Uniform, iter->second.TaskType) << expectedName;

			const auto& taskConfig = iter->second.Uniform;
			EXPECT_EQ(expectedStartDelay, taskConfig.StartDelay) << expectedName;
			EXPECT_EQ(expectedRepeatDelay, taskConfig.RepeatDelay) << expectedName;
		}

		void AssertContains(
				const TasksConfiguration& config,
				const std::string& expectedName,
				const TimeSpan& expectedStartDelay,
				const TimeSpan& expectedMinDelay,
				const TimeSpan& expectedMaxDelay,
				uint32_t expectedNumPhaseOneRounds,
				uint32_t expectedNumTransitionRounds) {
			// Assert:
			auto iter = config.Tasks.find(expectedName);
			ASSERT_TRUE(config.Tasks.cend() != iter) << expectedName;
			ASSERT_EQ(TasksConfiguration::TaskType::Decelerating, iter->second.TaskType) << expectedName;

			const auto& taskConfig = iter->second.Decelerating;
			EXPECT_EQ(expectedStartDelay, taskConfig.StartDelay) << expectedName;
			EXPECT_EQ(expectedMinDelay, taskConfig.MinDelay) << expectedName;
			EXPECT_EQ(expectedMaxDelay, taskConfig.MaxDelay) << expectedName;
			EXPECT_EQ(expectedNumPhaseOneRounds, taskConfig.NumPhaseOneRounds) << expectedName;
			EXPECT_EQ(expectedNumTransitionRounds, taskConfig.NumTransitionRounds) << expectedName;
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

	TEST(TEST_CLASS, Tasks_LoadFromBagCanLoadFromBagWithValues) {
		// Arrange: mix uniform and decelerating tasks
		utils::ConfigurationBag bag({
			{ "alpha", { { "startDelay", "20s" }, { "repeatDelay", "45s" } } },
			{
				"beta",
				{
					{ "startDelay", "11m" }, { "minDelay", "22m" }, { "maxDelay", "99m" },
					{ "numPhaseOneRounds", "14" }, { "numTransitionRounds", "123" }
				}
			},
			{ "gamma", { { "startDelay", "10s" }, { "repeatDelay", "2m" } } },
			{
				"eta",
				{
					{ "startDelay", "27m" }, { "minDelay", "34m" }, { "maxDelay", "35m" },
					{ "numPhaseOneRounds", "65" }, { "numTransitionRounds", "11" }
				}
			}
		});

		// Act:
		auto config = TasksConfiguration::LoadFromBag(bag);

		// Assert:
		EXPECT_EQ(4u, config.Tasks.size());
		AssertContains(config, "alpha", TimeSpan::FromSeconds(20), TimeSpan::FromSeconds(45));
		AssertContains(config, "beta", TimeSpan::FromMinutes(11), TimeSpan::FromMinutes(22), TimeSpan::FromMinutes(99), 14, 123);
		AssertContains(config, "gamma", TimeSpan::FromSeconds(10), TimeSpan::FromMinutes(2));
		AssertContains(config, "eta", TimeSpan::FromMinutes(27), TimeSpan::FromMinutes(34), TimeSpan::FromMinutes(35), 65, 11);
	}

	TEST(TEST_CLASS, Tasks_LoadFromBagFailsWhenAnyTaskHasInvalidValue) {
		// Arrange:
		utils::ConfigurationBag bag({
			{ "alpha", { { "startDelay", "20s" }, { "repeatDelay", "45s" } } },
			{ "beta", { { "startDelay", "11x" }, { "repeatDelay", "22m" } } },
			{ "gamma", { { "startDelay", "10s" }, { "repeatDelay", "2m" } } }
		});

		// Act + Assert:
		EXPECT_THROW(TasksConfiguration::LoadFromBag(bag), utils::property_malformed_error);
	}

	TEST(TEST_CLASS, Tasks_LoadFromBagFailsWhenAnyTaskHasMissingValue) {
		// Arrange:
		utils::ConfigurationBag bag({
			{ "alpha", { { "startDelay", "20s" }, { "repeatDelay", "45s" } } },
			{ "beta", { { "startDelay", "11m" } } },
			{ "gamma", { { "startDelay", "10s" }, { "repeatDelay", "2m" } } }
		});

		// Act + Assert:
		EXPECT_THROW(TasksConfiguration::LoadFromBag(bag), utils::property_not_found_error);
	}

	TEST(TEST_CLASS, Tasks_LoadFromBagFailsWhenAnyTaskHasExtraValue) {
		// Arrange:
		utils::ConfigurationBag bag({
			{ "alpha", { { "startDelay", "20s" }, { "repeatDelay", "45s" } } },
			{ "beta", { { "startDelay", "11m" }, { "minDelay", "15m" }, { "repeatDelay", "22m" } } },
			{ "gamma", { { "startDelay", "10s" }, { "repeatDelay", "2m" } } }
		});

		// Act + Assert:
		EXPECT_THROW(TasksConfiguration::LoadFromBag(bag), catapult_invalid_argument);
	}

	// endregion

	// region tasks (file io)

	TEST(TEST_CLASS, Tasks_LoadFromPathFailsWhenFileDoesNotExist) {
		// Act + Assert: attempt to load the config
		EXPECT_THROW(TasksConfiguration::LoadFromPath("../no-resources"), catapult_runtime_error);
	}

	TEST(TEST_CLASS, Tasks_CanLoadConfigFromResourcesDirectory) {
		// Act: attempt to load from the "real" resources directory
		auto config = TasksConfiguration::LoadFromPath("../resources");

		// Assert:
		EXPECT_EQ(19u, config.Tasks.size());

		// - spot check one task
		AssertContains(config, "harvesting task", TimeSpan::FromSeconds(30), TimeSpan::FromSeconds(1));
	}

	// endregion
}}
