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

		void AssertInvalidHarvestingConfiguration(
				const std::string& harvestKey,
				bool isAutoHarvestingEnabled,
				const std::string& beneficiary) {
			// Arrange:
			auto harvestingConfig = HarvestingConfiguration::Uninitialized();
			harvestingConfig.HarvestKey = harvestKey;
			harvestingConfig.IsAutoHarvestingEnabled = isAutoHarvestingEnabled;
			harvestingConfig.Beneficiary = beneficiary;

			// Act + Assert:
			EXPECT_THROW(ValidateHarvestingConfiguration(harvestingConfig), utils::property_malformed_error);
		}

		void AssertValidHarvestingConfiguration(
				const std::string& harvestKey,
				bool isAutoHarvestingEnabled,
				const std::string& beneficiary) {
			// Arrange:
			auto harvestingConfig = HarvestingConfiguration::Uninitialized();
			harvestingConfig.HarvestKey = harvestKey;
			harvestingConfig.IsAutoHarvestingEnabled = isAutoHarvestingEnabled;
			harvestingConfig.Beneficiary = beneficiary;

			// Act + Assert: no exception
			ValidateHarvestingConfiguration(harvestingConfig);
		}
	}

	// region harvest key

	TEST(TEST_CLASS, ValidationFailsWhenHarvestKeyIsInvalid) {
		AssertInvalidHarvestingConfiguration(Invalid_Key, true, Valid_Key);
		AssertInvalidHarvestingConfiguration(Invalid_Key, false, Valid_Key);
	}

	TEST(TEST_CLASS, ValidationFailsWhenHarvestKeyIsUnspecifiedAndAutoHarvestingIsEnabled) {
		AssertInvalidHarvestingConfiguration("", true, Valid_Key);
	}

	TEST(TEST_CLASS, ValidationSucceedsWhenHarvestKeyIsValidAndAutoHarvestingIsEnabled) {
		AssertValidHarvestingConfiguration(Valid_Key, true, Valid_Key);
	}

	TEST(TEST_CLASS, ValidationSucceedsWhenHarvestKeyIsUnspecifiedAndAutoHarvestingIsDisabled) {
		AssertValidHarvestingConfiguration("", false, Valid_Key);
	}

	// endregion

	// region beneficiary

	TEST(TEST_CLASS, ValidationFailsWhenBeneficiaryIsInvalid) {
		AssertInvalidHarvestingConfiguration(Valid_Key, false, Invalid_Key);
	}

	TEST(TEST_CLASS, ValidationSucceedsWhenBeneficiaryIsValid) {
		AssertValidHarvestingConfiguration(Valid_Key, false, Valid_Key);
	}

	// endregion
}}
