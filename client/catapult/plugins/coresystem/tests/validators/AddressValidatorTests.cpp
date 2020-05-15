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

	DEFINE_COMMON_VALIDATOR_TESTS(Address,)

	namespace {
		constexpr auto Network_Identifier = static_cast<model::NetworkIdentifier>(0xC8);

		Address GenerateRandomAddress() {
			return model::PublicKeyToAddress(test::GenerateRandomByteArray<Key>(), Network_Identifier);
		}

		model::ResolverContext CreateResolverContextXor() {
			auto xorResolver = test::CreateResolverContextXor();
			return model::ResolverContext(
					[xorResolver](const auto& unresolved) {
						return xorResolver.resolve(unresolved);
					},
					[xorResolver](const auto& unresolved) {
						// resolved addresses must have low bit cleared
						auto resolved = xorResolver.resolve(unresolved);
						resolved[0] ^= 0xFF;
						resolved[0] &= 0xFE;
						return resolved;
					});
		}

		UnresolvedAddress UnresolveXor(const Address& address) {
			auto unresolved = test::UnresolveXor(address);
			unresolved[0] ^= 0xFF;
			return unresolved;
		}

		void AssertValidationResult(ValidationResult expectedResult, const Address& address) {
			// Arrange: create validator context with custom resolver context
			auto networkInfo = model::NetworkInfo();
			networkInfo.Identifier = Network_Identifier;

			auto cache = test::CreateEmptyCatapultCache();
			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();

			auto notificationContext = model::NotificationContext(Height(1), CreateResolverContextXor());
			auto validatorContext = ValidatorContext(notificationContext, Timestamp(0), networkInfo, readOnlyCache);

			auto pValidator = CreateAddressValidator();
			model::AccountAddressNotification notification(UnresolveXor(address));

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, validatorContext);

			// Assert:
			EXPECT_EQ(expectedResult, result) << "address " << address;
		}
	}

	// region validation

	TEST(TEST_CLASS, SuccessWhenAddressIsCompatibleWithNetwork) {
		// Arrange:
		auto address = GenerateRandomAddress();

		// Assert:
		AssertValidationResult(ValidationResult::Success, address);
	}

	TEST(TEST_CLASS, SuccessWhenAddressIsCompatibleWithNetwork_Resolvable) {
		// Arrange:
		auto address = GenerateRandomAddress();
		address[0] |= 0x01;

		// Assert:
		AssertValidationResult(ValidationResult::Success, address);
	}

	TEST(TEST_CLASS, FailureWhenAddressIsIncompatibleWithNetwork) {
		// Arrange:
		auto address = GenerateRandomAddress();
		address[0] |= 0x02;

		// Assert:
		AssertValidationResult(Failure_Core_Invalid_Address, address);
	}

	TEST(TEST_CLASS, FailureWhenAddressIsIncompatibleWithNetwork_Resolvable) {
		// Arrange:
		auto address = GenerateRandomAddress();
		address[0] |= 0x03;

		// Assert:
		AssertValidationResult(Failure_Core_Invalid_Address, address);
	}

	TEST(TEST_CLASS, FailureWhenAddressHasInvalidChecksum) {
		// Arrange:
		auto address = GenerateRandomAddress();
		address[Address::Size / 2] ^= 0xFF;

		// Assert:
		AssertValidationResult(Failure_Core_Invalid_Address, address);
	}

	// endregion
}}
