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
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS EntityVersionValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(EntityVersion,)

	namespace {
		constexpr uint8_t Min_Entity_Version = 55;
		constexpr uint8_t Max_Entity_Version = 77;

		void AssertValidationResult(ValidationResult expectedResult, uint8_t version) {
			// Arrange:
			model::EntityNotification notification(
					model::NetworkIdentifier::Zero,
					static_cast<model::EntityType>(0x5A),
					version,
					Min_Entity_Version,
					Max_Entity_Version);
			auto pValidator = CreateEntityVersionValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "entity version " << static_cast<uint16_t>(version);
		}
	}

	// region validation

	TEST(TEST_CLASS, FailureWhenEntityHasVersionLowerThanMinVersion) {
		for (uint8_t version = 0u; version < Min_Entity_Version; ++version)
			AssertValidationResult(Failure_Core_Invalid_Version, version);
	}

	TEST(TEST_CLASS, FailureWhenEntityHasVersionGreaterThanMaxVersion) {
		for (uint8_t version = 0xFF; version >= Max_Entity_Version + 1; --version)
			AssertValidationResult(Failure_Core_Invalid_Version, version);
	}

	TEST(TEST_CLASS, SuccessWhenEntityHasVersionWithinBounds) {
		for (uint8_t version = Min_Entity_Version; version <= Max_Entity_Version; ++version)
			AssertValidationResult(ValidationResult::Success, version);
	}

	TEST(TEST_CLASS, SuccessWhenEntityMatchesBounds) {
		// Arrange:
		model::EntityNotification notification(model::NetworkIdentifier::Zero, static_cast<model::EntityType>(0x5A), 5, 5, 5);
		auto pValidator = CreateEntityVersionValidator();

		// Act:
		auto result = test::ValidateNotification(*pValidator, notification);

		// Assert:
		EXPECT_EQ(ValidationResult::Success, result);
	}

	// endregion
}}
