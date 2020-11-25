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
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS EntityForkVersionValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(EntityForkVersion, model::BlockChainForkHeights())

	namespace {
		constexpr auto Voting_Key_Link_Fork_Height = Height(2121);
		constexpr auto Block_Fork_Height = Height(3232);

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
			auto pValidator = CreateEntityForkVersionValidator({ Voting_Key_Link_Fork_Height, Block_Fork_Height, Height() });

			model::EntityNotification notification(model::NetworkIdentifier::Zero, entityType, entityVersion, 1, 100);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, validatorContext);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "entityType " << entityType << ", entityVersion " << entityVersion;
		}
	}

	// region validation - other

	TEST(TEST_CLASS, SuccessWhenEntityTypeIsNotMatch) {
		for (auto i = 0u; i < 4; ++i)
			AssertValidationResult(ValidationResult::Success, static_cast<model::EntityType>(i), 0, Height());
	}

	// endregion

	// region validation - voting key link

	namespace {
		void AssertVotingKeyLinkTransaction(ValidationResult expectedResult, uint8_t entityVersion, Height heightDelta) {
			auto contextHeight = Voting_Key_Link_Fork_Height + heightDelta;
			AssertValidationResult(expectedResult, model::Entity_Type_Voting_Key_Link, entityVersion, contextHeight);
		}
	}

	TEST(TEST_CLASS, FailureWhenVotingKeyLinkTransactionV2_BeforeFork) {
		AssertVotingKeyLinkTransaction(Failure_Core_Invalid_Version, 2, Height());
	}

	TEST(TEST_CLASS, SuccessWhenVotingKeyLinkTransactionV2_AfterFork) {
		AssertVotingKeyLinkTransaction(ValidationResult::Success, 2, Height(1));
	}

	TEST(TEST_CLASS, FailureWhenVotingKeyLinkTransactionV1_AfterFork) {
		AssertVotingKeyLinkTransaction(Failure_Core_Invalid_Version, 1, Height(1));
	}

	TEST(TEST_CLASS, FailureWhenVotingKeyLinkTransactionV1_BeforeFork) {
		AssertVotingKeyLinkTransaction(ValidationResult::Success, 1, Height(0));
	}

	// endregion

	// region validation - block

	namespace {
		void AssertBlock(ValidationResult expectedResult, uint8_t entityVersion, Height heightDelta) {
			auto contextHeight = Block_Fork_Height + heightDelta;
			AssertValidationResult(expectedResult, model::Entity_Type_Block_Normal, entityVersion, contextHeight);
			AssertValidationResult(expectedResult, model::Entity_Type_Block_Importance, entityVersion, contextHeight);
		}
	}

	TEST(TEST_CLASS, FailureWhenBlockV2_BeforeFork) {
		AssertBlock(Failure_Core_Invalid_Version, 2, Height());
	}

	TEST(TEST_CLASS, SuccessWhenBlockV2_AfterFork) {
		AssertBlock(ValidationResult::Success, 2, Height(1));
	}

	TEST(TEST_CLASS, FailureWhenBlockV1_AfterFork) {
		AssertBlock(Failure_Core_Invalid_Version, 1, Height(1));
	}

	TEST(TEST_CLASS, FailureWhenBlockV1_BeforeFork) {
		AssertBlock(ValidationResult::Success, 1, Height(0));
	}

	// endregion
}}
