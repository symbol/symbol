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

#include "src/validators/KeyLinkValidators.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS VotingKeyLinkRangeValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(VotingKeyLinkRange, 40, 100)

	namespace {
		constexpr auto Failure_Result = Failure_Core_Invalid_Link_Range;

		void AssertRangeValidatorTest(ValidationResult expectedResult, uint32_t startEpoch, uint32_t endEpoch) {
			// Arrange:
			auto pValidator = CreateVotingKeyLinkRangeValidator(40, 100);
			auto mainAccountPublicKey = test::GenerateRandomByteArray<Key>();
			auto votingPublicKey = test::GenerateRandomByteArray<VotingKey>();
			model::VotingKeyLinkNotification notification(
					mainAccountPublicKey,
					{ votingPublicKey, FinalizationEpoch(startEpoch), FinalizationEpoch(endEpoch) },
					static_cast<model::LinkAction>(test::RandomByte()));

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, ValidationFailsWhenStartIsZero) {
		AssertRangeValidatorTest(Failure_Core_Link_Start_Epoch_Invalid, 0, 49);
	}

	TEST(TEST_CLASS, ValidationFailsWhenStartIsLargerThanEnd) {
		AssertRangeValidatorTest(Failure_Result, 61, 60);
	}

	TEST(TEST_CLASS, ValidationFailsWhenStartIsLargerThanEndAndRangeIsWithinBounds) {
		AssertRangeValidatorTest(Failure_Result, 0xFFFFFFFF, 60);
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
