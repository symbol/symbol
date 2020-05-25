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
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS AddressAliasValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(AddressAlias,)

	namespace {
		template<typename TAccountIdentifier>
		auto CreateAndSeedCache(const TAccountIdentifier& accountIdentifier) {
			auto cache = test::CoreSystemCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			{
				auto cacheDelta = cache.createDelta();
				auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();
				accountStateCacheDelta.addAccount(accountIdentifier, Height());
				cache.commit(Height());
			}

			return cache;
		}

		void RunAddressAliasTest(ValidationResult expectedResult, const Address& cacheAddress, const Address& notificationAddress) {
			// Arrange:
			auto cache = CreateAndSeedCache(cacheAddress);

			auto pValidator = CreateAddressAliasValidator();
			auto notification = model::AliasedAddressNotification(NamespaceId(), model::AliasAction::Link, notificationAddress);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, FailureWhenAccountIsNotKnown) {
		// Arrange:
		auto address = test::GenerateRandomByteArray<Address>();
		auto unknownAddress = address;
		unknownAddress[0] ^= 0xFF;

		// Act + Assert:
		RunAddressAliasTest(Failure_Namespace_Alias_Invalid_Address, address, unknownAddress);
	}

	TEST(TEST_CLASS, SuccessWhenAccountIsKnown) {
		// Arrange:
		auto address = test::GenerateRandomByteArray<Address>();

		// Act + Assert:
		RunAddressAliasTest(ValidationResult::Success, address, address);
	}
}}
