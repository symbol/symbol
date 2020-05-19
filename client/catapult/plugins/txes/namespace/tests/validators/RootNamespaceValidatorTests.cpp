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

#include "src/validators/Validators.h"
#include "catapult/constants.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS RootNamespaceValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(RootNamespace, BlockDuration(), BlockDuration())

	// region duration

	namespace {
		void AssertDurationValidationResult(
				ValidationResult expectedResult,
				BlockDuration::ValueType duration,
				BlockDuration::ValueType minDuration,
				BlockDuration::ValueType maxDuration) {
			// Arrange:
			auto pValidator = CreateRootNamespaceValidator(BlockDuration(minDuration), BlockDuration(maxDuration));
			auto notification = model::RootNamespaceNotification(Address(), NamespaceId(), BlockDuration(duration));

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result)
					<< "duration " << duration
					<< ", minDuration " << minDuration
					<< ", maxDuration " << maxDuration;
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingRootNamespaceEternalDuration) {
		AssertDurationValidationResult(ValidationResult::Success, Eternal_Artifact_Duration.unwrap(), 111, 12345);
	}

	TEST(TEST_CLASS, FailureWhenValidatingRootNamespaceWithDurationLessThanMinDuration) {
		AssertDurationValidationResult(Failure_Namespace_Invalid_Duration, 1, 111, 12345);
		AssertDurationValidationResult(Failure_Namespace_Invalid_Duration, 100, 111, 12345);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingRootNamespaceWithDurationEqualToMinDuration) {
		AssertDurationValidationResult(ValidationResult::Success, 111, 111, 12345);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingRootNamespaceBetweenMinDurationAndMaxDuration) {
		AssertDurationValidationResult(ValidationResult::Success, 999, 111, 12345);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingRootNamespaceWithMaxDuration) {
		AssertDurationValidationResult(ValidationResult::Success, 12345, 111, 12345);
	}

	TEST(TEST_CLASS, FailureWhenValidatingRootNamespaceWithDurationGreaterThanMaxDuration) {
		AssertDurationValidationResult(Failure_Namespace_Invalid_Duration, 12346, 111, 12345);
		AssertDurationValidationResult(Failure_Namespace_Invalid_Duration, 55555, 111, 12345);
	}

	// endregion
}}
