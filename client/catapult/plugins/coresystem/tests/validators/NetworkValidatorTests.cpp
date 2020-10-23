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

#include "src/validators/Validators.h"
#include "catapult/model/VerifiableEntity.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS NetworkValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(Network, static_cast<model::NetworkIdentifier>(123))

	namespace {
		constexpr auto Network_Identifier = static_cast<model::NetworkIdentifier>(123);

		void AssertValidationResult(ValidationResult expectedResult, uint8_t networkIdentifier) {
			// Arrange:
			model::EntityNotification notification(
					static_cast<model::NetworkIdentifier>(networkIdentifier),
					static_cast<model::EntityType>(0),
					0,
					0,
					0);
			auto pValidator = CreateNetworkValidator(Network_Identifier);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "network identifier " << networkIdentifier;
		}
	}

	// region validation

	TEST(TEST_CLASS, SuccessWhenEntityHasSpecifiedCorrectNetwork) {
		AssertValidationResult(ValidationResult::Success, 123);
	}

	TEST(TEST_CLASS, FailureWhenEntityHasSpecifiedIncorrectNetwork) {
		for (uint8_t identifier = 0; identifier < 255; ++identifier) {
			if (123u == identifier)
				continue;

			AssertValidationResult(Failure_Core_Wrong_Network, identifier);
		}
	}

	// endregion
}}
