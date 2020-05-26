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
#include "catapult/model/Address.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS AccountAddressRestrictionNoSelfModificationValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(AccountAddressRestrictionNoSelfModification,)

	namespace {
		constexpr auto Add = model::AccountRestrictionModificationAction::Add;
		constexpr auto Del = model::AccountRestrictionModificationAction::Del;

		void AssertValidationResult(
				ValidationResult expectedResult,
				const Address& address,
				model::AccountRestrictionModificationAction action,
				UnresolvedAddress restrictionValue) {
			// Arrange:
			model::ModifyAccountAddressRestrictionValueNotification notification(
					address,
					model::AccountRestrictionFlags::Address,
					restrictionValue,
					action);
			auto pValidator = CreateAccountAddressRestrictionNoSelfModificationValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, test::CreateEmptyCatapultCache());

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, FailureWhenSignerIsValueInModification_Add) {
		auto address = test::GenerateRandomByteArray<Address>();
		auto unresolvedAddress = test::UnresolveXor(address);
		AssertValidationResult(Failure_RestrictionAccount_Invalid_Modification_Address, address, Add, unresolvedAddress);
	}

	TEST(TEST_CLASS, FailureWhenSignerIsValueInModification_Del) {
		auto address = test::GenerateRandomByteArray<Address>();
		auto unresolvedAddress = test::UnresolveXor(address);
		AssertValidationResult(Failure_RestrictionAccount_Invalid_Modification_Address, address, Del, unresolvedAddress);
	}

	TEST(TEST_CLASS, SuccessWhenSignerIsNotValueInModification) {
		auto address = test::GenerateRandomByteArray<Address>();
		AssertValidationResult(ValidationResult::Success, address, Add, test::GenerateRandomByteArray<UnresolvedAddress>());
	}
}}
