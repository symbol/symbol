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
#include "src/model/AggregateEntityType.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS AggregateTransactionVersionValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(AggregateTransactionVersion, Height(123), Height(234))

	namespace {
		void AssertValidationResult(
				ValidationResult expectedResult,
				model::EntityType entityType,
				uint8_t version,
				Height forkHeight1,
				Height forkHeight2,
				Height height) {
			// Arrange:
			cache::CatapultCache cache({});
			auto cacheView = cache.createView();
			auto validatorContext = test::CreateValidatorContext(height, cacheView.toReadOnly());

			model::EntityNotification notification(static_cast<model::NetworkIdentifier>(0), entityType, version, 0, 0);
			auto pValidator = CreateAggregateTransactionVersionValidator(forkHeight1, forkHeight2);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, validatorContext);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "height " << height;
		}

		void AssertNeverProhibited(model::EntityType entityType, uint8_t version) {
			AssertValidationResult(ValidationResult::Success, entityType, version, Height(100), Height(200), Height(50));
			AssertValidationResult(ValidationResult::Success, entityType, version, Height(100), Height(200), Height(99));
			AssertValidationResult(ValidationResult::Success, entityType, version, Height(100), Height(200), Height(100));
			AssertValidationResult(ValidationResult::Success, entityType, version, Height(100), Height(200), Height(101));
			AssertValidationResult(ValidationResult::Success, entityType, version, Height(100), Height(200), Height(150));
			AssertValidationResult(ValidationResult::Success, entityType, version, Height(100), Height(200), Height(199));
			AssertValidationResult(ValidationResult::Success, entityType, version, Height(100), Height(200), Height(200));
			AssertValidationResult(ValidationResult::Success, entityType, version, Height(100), Height(200), Height(201));
		}

		void AssertVersionOneProhibition(model::EntityType entityType, uint8_t version) {
			AssertValidationResult(ValidationResult::Success, entityType, version, Height(100), Height(200), Height(50));
			AssertValidationResult(ValidationResult::Success, entityType, version, Height(100), Height(200), Height(99));
			AssertValidationResult(Failure_Aggregate_V1_Prohibited, entityType, version, Height(100), Height(200), Height(100));
			AssertValidationResult(Failure_Aggregate_V1_Prohibited, entityType, version, Height(100), Height(200), Height(101));
			AssertValidationResult(Failure_Aggregate_V1_Prohibited, entityType, version, Height(100), Height(200), Height(150));
			AssertValidationResult(Failure_Aggregate_V1_Prohibited, entityType, version, Height(100), Height(200), Height(199));
			AssertValidationResult(Failure_Aggregate_V1_Prohibited, entityType, version, Height(100), Height(200), Height(200));
			AssertValidationResult(Failure_Aggregate_V1_Prohibited, entityType, version, Height(100), Height(200), Height(201));
		}

		void AssertVersionTwoProhibition(model::EntityType entityType, uint8_t version) {
			AssertValidationResult(Failure_Aggregate_V2_Prohibited, entityType, version, Height(100), Height(200), Height(50));
			AssertValidationResult(Failure_Aggregate_V2_Prohibited, entityType, version, Height(100), Height(200), Height(99));
			AssertValidationResult(ValidationResult::Success, entityType, version, Height(100), Height(200), Height(100));
			AssertValidationResult(ValidationResult::Success, entityType, version, Height(100), Height(200), Height(101));
			AssertValidationResult(ValidationResult::Success, entityType, version, Height(100), Height(200), Height(150));
			AssertValidationResult(ValidationResult::Success, entityType, version, Height(100), Height(200), Height(199));
			AssertValidationResult(Failure_Aggregate_V2_Prohibited, entityType, version, Height(100), Height(200), Height(200));
			AssertValidationResult(Failure_Aggregate_V2_Prohibited, entityType, version, Height(100), Height(200), Height(201));
		}

		void AssertVersionThreeProhibition(model::EntityType entityType, uint8_t version) {
			AssertValidationResult(Failure_Aggregate_V3_Prohibited, entityType, version, Height(100), Height(200), Height(50));
			AssertValidationResult(Failure_Aggregate_V3_Prohibited, entityType, version, Height(100), Height(200), Height(99));
			AssertValidationResult(Failure_Aggregate_V3_Prohibited, entityType, version, Height(100), Height(200), Height(100));
			AssertValidationResult(Failure_Aggregate_V3_Prohibited, entityType, version, Height(100), Height(200), Height(101));
			AssertValidationResult(Failure_Aggregate_V3_Prohibited, entityType, version, Height(100), Height(200), Height(150));
			AssertValidationResult(Failure_Aggregate_V3_Prohibited, entityType, version, Height(100), Height(200), Height(199));
			AssertValidationResult(ValidationResult::Success, entityType, version, Height(100), Height(200), Height(200));
			AssertValidationResult(ValidationResult::Success, entityType, version, Height(100), Height(200), Height(201));
		}
	}

	TEST(TEST_CLASS, OtherTransactionIsNeverProhibited) {
		AssertNeverProhibited(mocks::MockTransaction::Entity_Type, 1);
	}

	TEST(TEST_CLASS, AggregateCompleteV1IsAllowedBeforeFirstFork) {
		AssertVersionOneProhibition(model::Entity_Type_Aggregate_Complete, 1);
	}

	TEST(TEST_CLASS, AggregateBondedV1IsAllowedBeforeFirstFork) {
		AssertVersionOneProhibition(model::Entity_Type_Aggregate_Bonded, 1);
	}

	TEST(TEST_CLASS, AggregateCompleteV2IsAllowedBetweenForks) {
		AssertVersionTwoProhibition(model::Entity_Type_Aggregate_Complete, 2);
	}

	TEST(TEST_CLASS, AggregateBondedV2IsAllowedBetweenForks) {
		AssertVersionTwoProhibition(model::Entity_Type_Aggregate_Bonded, 2);
	}

	TEST(TEST_CLASS, AggregateCompleteV3IsAllowedAtAndAfterSecondFork) {
		AssertVersionThreeProhibition(model::Entity_Type_Aggregate_Complete, 3);
	}

	TEST(TEST_CLASS, AggregateBondedV3IsAllowedAtAndAfterSecondFork) {
		AssertVersionThreeProhibition(model::Entity_Type_Aggregate_Bonded, 3);
	}
}}
