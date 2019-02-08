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

#define TEST_CLASS HashLockMosaicValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(HashLockMosaic, UnresolvedMosaicId(11), Amount(123))

	namespace {
		constexpr UnresolvedMosaicId Currency_Mosaic_Id(1234);

		void AssertValidationResult(
				ValidationResult expectedResult,
				UnresolvedMosaicId mosaicId,
				Amount bondedAmount,
				Amount requiredBondedAmount) {
			// Arrange:
			model::UnresolvedMosaic mosaic{ mosaicId, bondedAmount };
			auto notification = model::HashLockMosaicNotification(mosaic);
			auto pValidator = CreateHashLockMosaicValidator(Currency_Mosaic_Id, requiredBondedAmount);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "notification with id " << mosaicId << " and amount " << bondedAmount;
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithProperMosaicIdAndAmount) {
		// Assert:
		AssertValidationResult(ValidationResult::Success, Currency_Mosaic_Id, Amount(500), Amount(500));
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithInvalidMosaicId) {
		// Assert:
		auto mosaicId = test::GenerateRandomValue<UnresolvedMosaicId>();
		AssertValidationResult(Failure_LockHash_Invalid_Mosaic_Id, mosaicId, Amount(500), Amount(500));
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithInvalidAmount) {
		// Assert:
		for (auto amount : { 0ull, 1ull, 10ull, 100ull, 499ull, 501ull, 1000ull })
			AssertValidationResult(Failure_LockHash_Invalid_Mosaic_Amount, UnresolvedMosaicId(123), Amount(amount), Amount(500));
	}
}}
