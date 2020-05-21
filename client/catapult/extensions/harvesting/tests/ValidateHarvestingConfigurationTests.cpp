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
		// the public / private key is invalid because it contains a non hex char ('G')
		const char* Invalid_Key = "3485D98EFD7EB07ADAFCFD1A157D89DE2G96A95E780813C0258AF3F5F84ED8CB";
		const char* Valid_Key = "3485D98EFD7EB07ADAFCFD1A157D89DE2796A95E780813C0258AF3F5F84ED8CB";

		HarvestingConfiguration CreateHarvestingConfiguration(
				const std::string& harvesterSigningPrivateKey,
				const std::string& harvesterVrfPrivateKey,
				bool enableAutoHarvesting) {
			auto harvestingConfig = HarvestingConfiguration::Uninitialized();
			harvestingConfig.HarvesterSigningPrivateKey = harvesterSigningPrivateKey;
			harvestingConfig.HarvesterVrfPrivateKey = harvesterVrfPrivateKey;
			harvestingConfig.EnableAutoHarvesting = enableAutoHarvesting;
			return harvestingConfig;
		}

		void AssertInvalidHarvestingConfiguration(const HarvestingConfiguration& harvestingConfig) {
			EXPECT_THROW(ValidateHarvestingConfiguration(harvestingConfig), utils::property_malformed_error);
		}

		void AssertValidHarvestingConfiguration(const HarvestingConfiguration& harvestingConfig) {
			EXPECT_NO_THROW(ValidateHarvestingConfiguration(harvestingConfig));
		}
	}

	// region harvester (signing|vrf) private key

	TEST(TEST_CLASS, ValidationFailsWhenAnyHarvesterPrivateKeyIsInvalid) {
		AssertInvalidHarvestingConfiguration(CreateHarvestingConfiguration(Invalid_Key, Valid_Key, true));
		AssertInvalidHarvestingConfiguration(CreateHarvestingConfiguration(Invalid_Key, Valid_Key, false));

		AssertInvalidHarvestingConfiguration(CreateHarvestingConfiguration(Valid_Key, Invalid_Key, true));
		AssertInvalidHarvestingConfiguration(CreateHarvestingConfiguration(Valid_Key, Invalid_Key, false));
	}

	TEST(TEST_CLASS, ValidationFailsWhenAnyHarvesterPrivateKeyIsUnspecifiedAndAutoHarvestingIsEnabled) {
		AssertInvalidHarvestingConfiguration(CreateHarvestingConfiguration("", Valid_Key, true));
		AssertInvalidHarvestingConfiguration(CreateHarvestingConfiguration(Valid_Key, "", true));
		AssertInvalidHarvestingConfiguration(CreateHarvestingConfiguration("", "", true));
	}

	TEST(TEST_CLASS, ValidationSucceedsWhenAllHarvesterPrivateKeysAreValid) {
		AssertValidHarvestingConfiguration(CreateHarvestingConfiguration(Valid_Key, Valid_Key, true));
		AssertValidHarvestingConfiguration(CreateHarvestingConfiguration(Valid_Key, Valid_Key, false));
	}

	TEST(TEST_CLASS, ValidationSucceedsWhenAnyHarvesterPrivateKeyIsUnspecifiedAndAutoHarvestingIsDisabled) {
		AssertValidHarvestingConfiguration(CreateHarvestingConfiguration("", Valid_Key, false));
		AssertValidHarvestingConfiguration(CreateHarvestingConfiguration(Valid_Key, "", false));
		AssertValidHarvestingConfiguration(CreateHarvestingConfiguration("", "", false));
	}

	// endregion
}}
