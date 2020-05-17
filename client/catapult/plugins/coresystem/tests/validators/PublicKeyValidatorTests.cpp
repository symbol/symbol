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
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/Address.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS PublicKeyValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(PublicKey,)

	namespace {
		constexpr auto Network_Identifier = static_cast<model::NetworkIdentifier>(0xC8);

		template<typename TIdentifier>
		void AddAccount(cache::CatapultCache& cache, const TIdentifier& identifier) {
			auto cacheDelta = cache.createDelta();
			cacheDelta.sub<cache::AccountStateCache>().addAccount(identifier, Height(1));
			cache.commit(Height(1));
		}

		void AssertValidationResult(ValidationResult expectedResult, const Key& publicKey, const cache::CatapultCache& cache) {
			auto networkInfo = model::NetworkInfo();
			networkInfo.Identifier = Network_Identifier;

			auto cacheView = cache.createView();
			auto readOnlyCache = cacheView.toReadOnly();

			auto validatorContext = test::CreateValidatorContext(Height(1), networkInfo, readOnlyCache);

			auto pValidator = CreatePublicKeyValidator();
			model::AccountPublicKeyNotification notification(publicKey);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, validatorContext);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, SuccessWhenPublicKeyAddressIsUnknown) {
		// Arrange:
		auto publicKey = test::GenerateRandomByteArray<Key>();

		auto cache = test::CreateEmptyCatapultCache();
		AddAccount(cache, publicKey);

		// Act + Assert:
		AssertValidationResult(ValidationResult::Success, test::GenerateRandomByteArray<Key>(), std::move(cache));
	}

	TEST(TEST_CLASS, SuccessWhenPublicKeyAddressIsKnownWithoutPublicKey) {
		// Arrange:
		auto publicKey = test::GenerateRandomByteArray<Key>();
		auto address = model::PublicKeyToAddress(publicKey, Network_Identifier);

		auto cache = test::CreateEmptyCatapultCache();
		AddAccount(cache, address);

		// Act + Assert:
		AssertValidationResult(ValidationResult::Success, publicKey, std::move(cache));
	}

	TEST(TEST_CLASS, SuccessWhenPublicKeyAddressIsKnownWithMatchingPublicKey) {
		// Arrange:
		auto publicKey = test::GenerateRandomByteArray<Key>();

		auto cache = test::CreateEmptyCatapultCache();
		AddAccount(cache, publicKey);

		// Act + Assert:
		AssertValidationResult(ValidationResult::Success, publicKey, std::move(cache));
	}

	// NOTE: failure case is omitted because it requires a RIPEMD160 collision
}}
