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
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS VotingKeyLinkRangeValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(VotingKeyLinkRange, 40, 100)

	namespace {
		constexpr auto Failure_Result = Failure_Core_Invalid_Link_Range;

		void AssertRangeValidatorTest(ValidationResult expectedResult, uint64_t startPoint, uint64_t endPoint) {
			// Arrange:
			auto pValidator = CreateVotingKeyLinkRangeValidator(40, 100);
			auto mainAccountPublicKey = test::GenerateRandomByteArray<Key>();
			auto votingPublicKey = test::GenerateRandomByteArray<VotingKey>();
			model::VotingKeyLinkNotification notification(
					mainAccountPublicKey,
					{ votingPublicKey, FinalizationPoint(startPoint), FinalizationPoint(endPoint) },
					static_cast<model::LinkAction>(test::RandomByte()));

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, ValidationFailsWhenStartIsLargerThanEnd) {
		AssertRangeValidatorTest(Failure_Result, 61, 60);
	}

	TEST(TEST_CLASS, ValidationFailsWhenStartIsLargerThanEndAndRangeIsWithinBounds) {
		AssertRangeValidatorTest(Failure_Result, 0xFFFFFFFF'FFFFFFFF, 60);
	}

	TEST(TEST_CLASS, ValidationFailsWhenRangeIsTooShort) {
		AssertRangeValidatorTest(Failure_Result, 10, 48);
	}

	TEST(TEST_CLASS, ValidationSucceedsWhenRangeIsAtMinimumBoundary) {
		AssertRangeValidatorTest(ValidationResult::Success, 10, 49);
	}

	TEST(TEST_CLASS, ValidationSucceedsWhenRangeIsWithinBoundaries) {
		AssertRangeValidatorTest(ValidationResult::Success, 10, 70);
	}

	TEST(TEST_CLASS, ValidationSucceedsWhenRangeIsAtMaximumBoundary) {
		AssertRangeValidatorTest(ValidationResult::Success, 10, 109);
	}

	TEST(TEST_CLASS, ValidationFailsWhenRangeIsTooLong) {
		AssertRangeValidatorTest(Failure_Result, 10, 110);
	}
}}
