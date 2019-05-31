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

#define TEST_CLASS AddressValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(Address, static_cast<model::NetworkIdentifier>(123))

	namespace {
		constexpr auto Network_Identifier = static_cast<model::NetworkIdentifier>(123);

		void AssertValidationResult(ValidationResult expectedResult, const Address& address) {
			// Arrange:
			auto cache = test::CreateEmptyCatapultCache();

			model::AccountAddressNotification notification(test::UnresolveXor(address));
			auto pValidator = CreateAddressValidator(Network_Identifier);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "address " << address;
		}
	}

	// region validation

	TEST(TEST_CLASS, SuccessWhenAddressIsCompatibleWithNetwork) {
		// Arrange:
		auto address = PublicKeyToAddress(test::GenerateRandomByteArray<Key>(), Network_Identifier);

		// Assert:
		AssertValidationResult(ValidationResult::Success, address);
	}

	TEST(TEST_CLASS, FailureWhenAddressHasInvalidChecksum) {
		// Arrange:
		auto address = PublicKeyToAddress(test::GenerateRandomByteArray<Key>(), Network_Identifier);
		address[Address_Decoded_Size / 2] ^= 0xFF;

		// Assert:
		AssertValidationResult(Failure_Core_Invalid_Address, address);
	}

	TEST(TEST_CLASS, FailureWhenAddressIsIncompatibleWithNetwork) {
		// Arrange:
		auto address = PublicKeyToAddress(test::GenerateRandomByteArray<Key>(), model::NetworkIdentifier::Mijin_Test);

		// Assert:
		AssertValidationResult(Failure_Core_Invalid_Address, address);
	}

	// endregion
}}
