/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "timesync/src/TimeSynchronizationConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace timesync {

#define TEST_CLASS TimeSynchronizationConfigurationTests

	namespace {
		struct TimeSynchronizationConfigurationTraits {
			using ConfigurationType = TimeSynchronizationConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"timesynchronization",
						{
							{ "maxNodes", "123" },
							{ "minImportance", "987" }
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const TimeSynchronizationConfiguration& config) {
				// Assert:
				EXPECT_EQ(0u, config.MaxNodes);
				EXPECT_EQ(Importance(0), config.MinImportance);
			}

			static void AssertCustom(const TimeSynchronizationConfiguration& config) {
				// Assert:
				EXPECT_EQ(123u, config.MaxNodes);
				EXPECT_EQ(Importance(987), config.MinImportance);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(TimeSynchronizationConfigurationTests, TimeSynchronization)

	// region file io

	TEST(TEST_CLASS, LoadFromPathFailsWhenFileDoesNotExist) {
		// Act + Assert: attempt to load the config
		EXPECT_THROW(TimeSynchronizationConfiguration::LoadFromPath("../no-resources"), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanLoadConfigFromResourcesDirectory) {
		// Act: attempt to load from the "real" resources directory
		auto config = TimeSynchronizationConfiguration::LoadFromPath("../resources");

		// Assert:
		EXPECT_EQ(20u, config.MaxNodes);
		EXPECT_EQ(Importance(3'750), config.MinImportance);
	}

	// endregion
}}
