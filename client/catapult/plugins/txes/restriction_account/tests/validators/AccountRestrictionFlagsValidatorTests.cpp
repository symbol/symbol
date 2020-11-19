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

#define TEST_CLASS AccountRestrictionFlagsValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(AccountRestrictionFlags,)

	namespace {
		void AssertValidationResult(ValidationResult expectedResult, model::AccountRestrictionFlags restrictionFlags) {
			// Arrange:
			model::AccountRestrictionModificationNotification notification(restrictionFlags, 0, 0);
			auto pValidator = CreateAccountRestrictionFlagsValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "notification with restriction flags " << utils::to_underlying_type(restrictionFlags);
		}

		void AssertValidTypes(const std::vector<model::AccountRestrictionFlags>& restrictionFlagsContainer) {
			for (auto restrictionFlags : restrictionFlagsContainer) {
				AssertValidationResult(ValidationResult::Success, restrictionFlags);
				AssertValidationResult(ValidationResult::Success, restrictionFlags | model::AccountRestrictionFlags::Block);
			}
		}

		void AssertInvalidTypes(const std::vector<model::AccountRestrictionFlags>& restrictionFlagsContainer) {
			constexpr auto Invalid_Type = Failure_RestrictionAccount_Invalid_Restriction_Flags;
			for (auto restrictionFlags : restrictionFlagsContainer) {
				AssertValidationResult(Invalid_Type, restrictionFlags);
				AssertValidationResult(Invalid_Type, restrictionFlags | model::AccountRestrictionFlags::Block);
			}
		}
	}

	TEST(TEST_CLASS, SuccessWhenValidatingNotificationWithKnownAccountRestrictionFlags) {
		AssertValidTypes({
				model::AccountRestrictionFlags::Address,
				model::AccountRestrictionFlags::Address | model::AccountRestrictionFlags::Outgoing,
				model::AccountRestrictionFlags::MosaicId,
				model::AccountRestrictionFlags::TransactionType | model::AccountRestrictionFlags::Outgoing
		});
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithUnknownAccountRestrictionFlags) {
		AssertValidationResult(Failure_RestrictionAccount_Invalid_Restriction_Flags, model::AccountRestrictionFlags::Sentinel);
		AssertValidationResult(Failure_RestrictionAccount_Invalid_Restriction_Flags, static_cast<model::AccountRestrictionFlags>(0x10));
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithNoFlagsSet) {
		AssertValidationResult(Failure_RestrictionAccount_Invalid_Restriction_Flags, static_cast<model::AccountRestrictionFlags>(0));
	}

	TEST(TEST_CLASS, FailureWhenValidatingNotificationWithMultipleFlagsSet) {
		AssertInvalidTypes({
			model::AccountRestrictionFlags::MosaicId | model::AccountRestrictionFlags::Outgoing,
			model::AccountRestrictionFlags::TransactionType,
			static_cast<model::AccountRestrictionFlags>(3),
			static_cast<model::AccountRestrictionFlags>(3) | model::AccountRestrictionFlags::Outgoing,
			static_cast<model::AccountRestrictionFlags>(7),
			static_cast<model::AccountRestrictionFlags>(7) | model::AccountRestrictionFlags::Outgoing,
			static_cast<model::AccountRestrictionFlags>(0xFF)
		});
	}
}}
