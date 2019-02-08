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

#include "harvesting/src/ValidateHarvestingConfiguration.h"
#include "harvesting/src/HarvestingConfiguration.h"
#include "catapult/utils/ConfigurationBag.h"
#include "tests/TestHarness.h"

namespace catapult { namespace harvesting {

#define TEST_CLASS ValidateHarvestingConfigurationTests

	namespace {
		// the key is invalid because it contains a non hex char ('G')
		const char* Invalid_Private_Key = "3485d98efd7eb07adafcfd1a157d89de2G96a95e780813c0258af3f5f84ed8cb";
		const char* Valid_Private_Key = "3485d98efd7eb07adafcfd1a157d89de2796a95e780813c0258af3f5f84ed8cb";

		void AssertInvalidHarvestingConfiguration(const std::string& harvestKey, bool isAutoHarvestingEnabled) {
			// Arrange:
			auto harvestingConfig = HarvestingConfiguration::Uninitialized();
			harvestingConfig.HarvestKey = harvestKey;
			harvestingConfig.IsAutoHarvestingEnabled = isAutoHarvestingEnabled;

			// Act + Assert:
			EXPECT_THROW(ValidateHarvestingConfiguration(harvestingConfig), utils::property_malformed_error);
		}

		void AssertValidHarvestingConfiguration(const std::string& harvestKey, bool isAutoHarvestingEnabled) {
			// Arrange:
			auto harvestingConfig = HarvestingConfiguration::Uninitialized();
			harvestingConfig.HarvestKey = harvestKey;
			harvestingConfig.IsAutoHarvestingEnabled = isAutoHarvestingEnabled;

			// Act + Assert: no exception
			ValidateHarvestingConfiguration(harvestingConfig);
		}
	}

	TEST(TEST_CLASS, ValidationFailsIfHarvestKeyIsInvalid) {
		// Assert:
		AssertInvalidHarvestingConfiguration(Invalid_Private_Key, true);
		AssertInvalidHarvestingConfiguration(Invalid_Private_Key, false);
	}

	TEST(TEST_CLASS, ValidationFailsIfHarvestKeyIsUnspecifiedAndAutoHarvestingIsEnabled) {
		// Assert:
		AssertInvalidHarvestingConfiguration("", true);
	}

	TEST(TEST_CLASS, ValidationSucceedsIfHarvestKeyIsValidAndAutoHarvestingIsEnabled) {
		// Assert:
		AssertValidHarvestingConfiguration(Valid_Private_Key, true);
	}

	TEST(TEST_CLASS, ValidationSucceedsIfHarvestKeyIsUnspecifiedAndAutoHarvestingIsDisabled) {
		// Assert:
		AssertValidHarvestingConfiguration("", false);
	}
}}
