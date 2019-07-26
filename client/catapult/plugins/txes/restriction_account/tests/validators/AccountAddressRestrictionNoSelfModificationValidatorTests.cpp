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
#include "sdk/src/extensions/ConversionExtensions.h"
#include "catapult/model/Address.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS AccountAddressRestrictionNoSelfModificationValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(AccountAddressRestrictionNoSelfModification, model::NetworkIdentifier::Zero)

	namespace {
		constexpr auto Add = model::AccountRestrictionModificationType::Add;
		constexpr auto Del = model::AccountRestrictionModificationType::Del;

		void AssertValidationResult(
				ValidationResult expectedResult,
				const Key& signer,
				const model::AccountRestrictionModification<UnresolvedAddress>& modification) {
			// Arrange:
			model::ModifyAccountAddressRestrictionValueNotification notification(
					signer,
					model::AccountRestrictionType::Address,
					modification);
			auto pValidator = CreateAccountAddressRestrictionNoSelfModificationValidator(model::NetworkIdentifier::Zero);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, FailureWhenSignerIsValueInModification_Add) {
		auto key = test::GenerateRandomByteArray<Key>();
		auto address = model::PublicKeyToAddress(key, model::NetworkIdentifier::Zero);
		AssertValidationResult(Failure_RestrictionAccount_Modification_Address_Invalid, key, {
			Add,
			extensions::CopyToUnresolvedAddress(address)
		});
	}

	TEST(TEST_CLASS, FailureWhenSignerIsValueInModification_Del) {
		auto key = test::GenerateRandomByteArray<Key>();
		auto address = model::PublicKeyToAddress(key, model::NetworkIdentifier::Zero);
		AssertValidationResult(Failure_RestrictionAccount_Modification_Address_Invalid, key, {
			Del,
			extensions::CopyToUnresolvedAddress(address)
		});
	}

	TEST(TEST_CLASS, SuccessWhenSignerIsNotValueInModification) {
		auto key = test::GenerateRandomByteArray<Key>();
		auto address = test::GenerateRandomByteArray<Address>();
		AssertValidationResult(ValidationResult::Success, key, { Add, extensions::CopyToUnresolvedAddress(address) });
	}
}}
