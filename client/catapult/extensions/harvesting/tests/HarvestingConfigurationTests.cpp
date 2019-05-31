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

#include "harvesting/src/HarvestingConfiguration.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace harvesting {

#define TEST_CLASS HarvestingConfigurationTests

	namespace {
		struct HarvestingConfigurationTraits {
			using ConfigurationType = HarvestingConfiguration;

			static utils::ConfigurationBag::ValuesContainer CreateProperties() {
				return {
					{
						"harvesting",
						{
							{ "harvestKey", "harvest-key" },
							{ "isAutoHarvestingEnabled", "true" },
							{ "maxUnlockedAccounts", "2" },
							{ "beneficiary", "beneficiary-key" }
						}
					}
				};
			}

			static bool IsSectionOptional(const std::string&) {
				return false;
			}

			static void AssertZero(const HarvestingConfiguration& config) {
				// Assert:
				EXPECT_EQ("", config.HarvestKey);
				EXPECT_FALSE(config.IsAutoHarvestingEnabled);
				EXPECT_EQ(0u, config.MaxUnlockedAccounts);
				EXPECT_EQ("", config.Beneficiary);
			}

			static void AssertCustom(const HarvestingConfiguration& config) {
				// Assert:
				EXPECT_EQ("harvest-key", config.HarvestKey);
				EXPECT_TRUE(config.IsAutoHarvestingEnabled);
				EXPECT_EQ(2u, config.MaxUnlockedAccounts);
				EXPECT_EQ("beneficiary-key", config.Beneficiary);
			}
		};
	}

	DEFINE_CONFIGURATION_TESTS(HarvestingConfigurationTests, Harvesting)

	// region file io

	TEST(TEST_CLASS, LoadFromPathFailsWhenFileDoesNotExist) {
		// Act + Assert: attempt to load the config
		EXPECT_THROW(HarvestingConfiguration::LoadFromPath("../no-resources"), catapult_runtime_error);
	}

	TEST(TEST_CLASS, CanLoadConfigFromResourcesDirectory) {
		// Act: attempt to load from the "real" resources directory
		auto config = HarvestingConfiguration::LoadFromPath("../resources");

		// Assert:
		EXPECT_EQ("", config.HarvestKey);
		EXPECT_FALSE(config.IsAutoHarvestingEnabled);
		EXPECT_EQ(5u, config.MaxUnlockedAccounts);
		EXPECT_EQ("0000000000000000000000000000000000000000000000000000000000000000", config.Beneficiary);
	}

	// endregion
}}
