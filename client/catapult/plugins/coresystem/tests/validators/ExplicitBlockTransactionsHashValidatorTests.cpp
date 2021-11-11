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
#include "catapult/validators/ValidatorContext.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/NotificationTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

	DEFINE_COMMON_VALIDATOR_TESTS(ExplicitBlockTransactionsHash, Height(), Hash256())

#define TEST_CLASS ExplicitBlockTransactionsHashTests

	namespace {
		constexpr auto Success_Result = ValidationResult::Success;
		constexpr auto Failure_Result = Failure_Core_Block_Explicit_Transactions_Hash_Mismatch;
		constexpr auto Fork_Height = Height(1000);

		enum class TransactionsHashMatchMode { Enabled, Disabled };

		void AssertValidationResult(ValidationResult expectedResult, Height height, TransactionsHashMatchMode transactionsHashMatchMode) {
			// Arrange:
			auto cache = test::CreateEmptyCatapultCache();
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();

			auto expectedTransactionsHash = test::GenerateRandomByteArray<Hash256>();
			auto pValidator = CreateExplicitBlockTransactionsHashValidator(Fork_Height, expectedTransactionsHash);
			auto context = test::CreateValidatorContext(height, readOnlyCache);

			auto notification = test::CreateBlockNotification();
			notification.TransactionsHash = TransactionsHashMatchMode::Enabled == transactionsHashMatchMode
					? expectedTransactionsHash
					: test::GenerateRandomByteArray<Hash256>();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, context);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingBlockNotAtForkHeightWithMatchingTransaction) {
		AssertValidationResult(Success_Result, Fork_Height - Height(100), TransactionsHashMatchMode::Enabled);
		AssertValidationResult(Success_Result, Fork_Height - Height(1), TransactionsHashMatchMode::Enabled);
		AssertValidationResult(Success_Result, Fork_Height + Height(1), TransactionsHashMatchMode::Enabled);
		AssertValidationResult(Success_Result, Fork_Height + Height(100), TransactionsHashMatchMode::Enabled);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingBlockAtForkHeightWithMatchingTransaction) {
		AssertValidationResult(Success_Result, Fork_Height, TransactionsHashMatchMode::Enabled);
	}

	TEST(TEST_CLASS, SuccessWhenValidatingBlockNotAtForkHeightWithoutMatchingTransaction) {
		AssertValidationResult(Success_Result, Fork_Height - Height(100), TransactionsHashMatchMode::Disabled);
		AssertValidationResult(Success_Result, Fork_Height - Height(1), TransactionsHashMatchMode::Disabled);
		AssertValidationResult(Success_Result, Fork_Height + Height(1), TransactionsHashMatchMode::Disabled);
		AssertValidationResult(Success_Result, Fork_Height + Height(100), TransactionsHashMatchMode::Disabled);
	}

	TEST(TEST_CLASS, FailureWhenValidatingBlockAtForkHeightWithoutMatchingTransaction) {
		AssertValidationResult(Failure_Result, Fork_Height, TransactionsHashMatchMode::Disabled);
	}
}}
