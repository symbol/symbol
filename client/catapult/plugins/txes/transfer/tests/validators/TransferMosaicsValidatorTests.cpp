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

#define TEST_CLASS TransferMosaicsValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(TransferMosaics,)

	namespace {
		constexpr auto Success_Result = ValidationResult::Success;

		void AssertValidationResult(ValidationResult expectedResult, const std::vector<model::UnresolvedMosaic>& mosaics) {
			// Arrange:
			model::TransferMosaicsNotification notification(static_cast<uint8_t>(mosaics.size()), mosaics.data());
			auto pValidator = CreateTransferMosaicsValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithZeroMosaics) {
		AssertValidationResult(Success_Result, {});
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithOneMosaic) {
		AssertValidationResult(Success_Result, { { UnresolvedMosaicId(71), Amount(5) } });
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithMultipleOrderedMosaics) {
		AssertValidationResult(
				Success_Result,
				{ { UnresolvedMosaicId(71), Amount(5) }, { UnresolvedMosaicId(182), Amount(4) }, { UnresolvedMosaicId(200), Amount(1) } });
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithMultipleOutOfOrderMosaics) {
		// Assert: first and second are out of order
		AssertValidationResult(
				Failure_Transfer_Out_Of_Order_Mosaics,
				{ { UnresolvedMosaicId(200), Amount(5) }, { UnresolvedMosaicId(71), Amount(1) }, { UnresolvedMosaicId(182), Amount(4) } });

		// - second and third are out of order
		AssertValidationResult(
				Failure_Transfer_Out_Of_Order_Mosaics,
				{ { UnresolvedMosaicId(71), Amount(5) }, { UnresolvedMosaicId(200), Amount(1) }, { UnresolvedMosaicId(182), Amount(4) } });
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithMultipleTransfersOfSameMosaic) {
		// Assert: create a transaction with multiple (in order) transfers for the same mosaic
		AssertValidationResult(
				Failure_Transfer_Out_Of_Order_Mosaics,
				{
					{ UnresolvedMosaicId(71), Amount(5) },
					{ UnresolvedMosaicId(182), Amount(4) },
					{ UnresolvedMosaicId(182), Amount(4) },
					{ UnresolvedMosaicId(200), Amount(1) }
				});
	}
}}
