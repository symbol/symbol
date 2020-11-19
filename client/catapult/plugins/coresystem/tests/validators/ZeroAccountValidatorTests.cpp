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
#include "sdk/src/extensions/ConversionExtensions.h"
#include "catapult/model/Address.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

	DEFINE_COMMON_VALIDATOR_TESTS(ZeroAddress, model::NetworkIdentifier::Zero)
	DEFINE_COMMON_VALIDATOR_TESTS(ZeroPublicKey,)

#define ADDRESS_TEST_CLASS ZeroAddressValidatorTests
#define PUBLIC_KEY_TEST_CLASS ZeroPublicKeyValidatorTests

	// region Address

	namespace {
		void RunAddressTest(ValidationResult expectedResult, const Address& address) {
			// Arrange:
			auto pValidator = CreateZeroAddressValidator(static_cast<model::NetworkIdentifier>(123));

			// Act:
			auto unresolvedAddress = extensions::CopyToUnresolvedAddress(address);
			auto result = test::ValidateNotification(*pValidator, model::AccountAddressNotification(unresolvedAddress));

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(ADDRESS_TEST_CLASS, SuccessWhenAddressIsDerivedFromNonzeroKey) {
		auto address = model::PublicKeyToAddress({ { 1 } }, static_cast<model::NetworkIdentifier>(123));
		RunAddressTest(ValidationResult::Success, address);
	}

	TEST(ADDRESS_TEST_CLASS, SuccessWhenAddressIsDerivedFromZeroKeyWithDifferentNetwork) {
		auto address = model::PublicKeyToAddress(Key(), static_cast<model::NetworkIdentifier>(122));
		RunAddressTest(ValidationResult::Success, address);
	}

	TEST(ADDRESS_TEST_CLASS, FailureWhenAddressIsDerivedFromZeroKeyWithMatchingNetwork) {
		auto address = model::PublicKeyToAddress(Key(), static_cast<model::NetworkIdentifier>(123));
		RunAddressTest(Failure_Core_Zero_Address, address);
	}

	// endregion

	// region PublicKey

	namespace {
		void RunPublicKeyTest(ValidationResult expectedResult, const Key& publicKey) {
			// Arrange:
			auto pValidator = CreateZeroPublicKeyValidator();

			// Act:
			auto result = test::ValidateNotification(*pValidator, model::AccountPublicKeyNotification(publicKey));

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(PUBLIC_KEY_TEST_CLASS, SuccessWhenPublicKeyIsNonzero) {
		RunPublicKeyTest(ValidationResult::Success, { { 1 } });
	}

	TEST(PUBLIC_KEY_TEST_CLASS, FailureWhenPublicKeyIsZero) {
		RunPublicKeyTest(Failure_Core_Zero_Public_Key, Key());
	}

	// endregion
}}
