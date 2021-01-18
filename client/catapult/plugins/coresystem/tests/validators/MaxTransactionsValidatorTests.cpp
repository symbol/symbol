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
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS MaxTransactionsValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(MaxTransactions, 123)

	namespace {
		constexpr uint32_t Max_Transactions = 10;
		constexpr auto Success_Result = ValidationResult::Success;
		constexpr auto Failure_Result = Failure_Core_Too_Many_Transactions;

		void AssertValidationResult(ValidationResult expectedResult, model::EntityType blockType, uint32_t numTransactions) {
			// Arrange:
			auto notification = test::CreateBlockNotification();
			notification.BlockType = blockType;
			notification.NumTransactions = numTransactions;
			auto pValidator = CreateMaxTransactionsValidator(Max_Transactions);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << blockType << ", numTransactions " << numTransactions;
		}
	}

	// region validation

	TEST(TEST_CLASS, SuccessWhenBlockContainsNoTransactions) {
		for (auto blockType : { model::Entity_Type_Block_Nemesis, model::Entity_Type_Block_Normal, model::Entity_Type_Block_Importance })
			AssertValidationResult(Success_Result, blockType, 0);
	}

	TEST(TEST_CLASS, SuccessWhenBlockContainsLessThanMaxTransactions) {
		for (auto blockType : { model::Entity_Type_Block_Nemesis, model::Entity_Type_Block_Normal, model::Entity_Type_Block_Importance }) {
			AssertValidationResult(Success_Result, blockType, 1);
			AssertValidationResult(Success_Result, blockType, 5);
			AssertValidationResult(Success_Result, blockType, Max_Transactions - 1);
		}
	}

	TEST(TEST_CLASS, SuccessWhenBlockContainsMaxTransactions) {
		for (auto blockType : { model::Entity_Type_Block_Nemesis, model::Entity_Type_Block_Normal, model::Entity_Type_Block_Importance })
			AssertValidationResult(Success_Result, blockType, Max_Transactions);
	}

	TEST(TEST_CLASS, SuccessWhenBlockContainsMoreThanMaxTransactions_Nemesis) {
		auto blockType = model::Entity_Type_Block_Nemesis;
		AssertValidationResult(Success_Result, blockType, Max_Transactions + 1);
		AssertValidationResult(Success_Result, blockType, Max_Transactions + 10);
		AssertValidationResult(Success_Result, blockType, Max_Transactions + 100);
	}

	TEST(TEST_CLASS, FailureWhenBlockContainsMoreThanMaxTransactions_NonNemesis) {
		for (auto blockType : { model::Entity_Type_Block_Normal, model::Entity_Type_Block_Importance }) {
			AssertValidationResult(Failure_Result, blockType, Max_Transactions + 1);
			AssertValidationResult(Failure_Result, blockType, Max_Transactions + 10);
			AssertValidationResult(Failure_Result, blockType, Max_Transactions + 100);
		}
	}

	// endregion
}}
