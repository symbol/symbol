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
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS EntityForkVersionValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(EntityForkVersion, Height())

	namespace {
		constexpr auto Valid_Entity_Type = model::Entity_Type_Voting_Key_Link;
		constexpr auto Fork_Height = Height(12345);

		void AssertValidationResult(
				ValidationResult expectedResult,
				model::EntityType entityType,
				uint8_t entityVersion,
				Height contextHeight) {
			// Arrange:
			auto cache = test::CreateEmptyCatapultCache();
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();
			auto validatorContext = test::CreateValidatorContext(contextHeight, readOnlyCache);
			auto pValidator = CreateEntityForkVersionValidator(Fork_Height);

			model::EntityNotification notification(model::NetworkIdentifier::Zero, entityType, entityVersion, 1, 100);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, validatorContext);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "entityType " << entityType << ", entityVersion " << entityVersion;
		}
	}

	// region basic tests

	TEST(TEST_CLASS, SuccessWhenTransactionTypeIsNotVotingKeyLink) {
		for (auto i = 0u; i < 4; ++i)
			AssertValidationResult(ValidationResult::Success, static_cast<model::EntityType>(i), 0, Height());
	}

	TEST(TEST_CLASS, FailureWhenVotingTransactionV2_BeforeFork) {
		AssertValidationResult(Failure_Core_Invalid_Version, Valid_Entity_Type, 2, Fork_Height);
	}

	TEST(TEST_CLASS, SuccessWhenVotingTransactionV2_AfterFork) {
		AssertValidationResult(ValidationResult::Success, Valid_Entity_Type, 2, Fork_Height + Height(1));
	}

	TEST(TEST_CLASS, FailureWhenVotingTransactionV1_AfterFork) {
		AssertValidationResult(Failure_Core_Invalid_Version, Valid_Entity_Type, 1, Fork_Height + Height(1));
	}

	TEST(TEST_CLASS, FailureWhenVotingTransactionV1_BeforeFork) {
		AssertValidationResult(ValidationResult::Success, Valid_Entity_Type, 1, Fork_Height);
	}

	// endregion
}}
