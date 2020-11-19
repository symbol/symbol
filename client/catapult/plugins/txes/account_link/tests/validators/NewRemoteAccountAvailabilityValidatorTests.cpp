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
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS NewRemoteAccountAvailabilityValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(NewRemoteAccountAvailability,)

	namespace {
		cache::CatapultCache CreateCache(const Key& publicKey, state::AccountType accountType) {
			auto cache = test::CoreSystemCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			auto cacheDelta = cache.createDelta();
			auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();
			accountStateCacheDelta.addAccount(publicKey, Height(1));
			accountStateCacheDelta.find(publicKey).get().AccountType = accountType;
			cache.commit(Height(1));
			return cache;
		}

		void AssertValidation(ValidationResult expectedResult, const cache::CatapultCache& cache, const Key& publicKey) {
			// Arrange:
			auto pValidator = CreateNewRemoteAccountAvailabilityValidator();
			auto notification = model::NewRemoteAccountNotification(publicKey);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

		void AssertFailsWhenAccountExistsWithType(state::AccountType accountType) {
			// Arrange:
			auto publicKey = test::GenerateRandomByteArray<Key>();
			auto cache = CreateCache(publicKey, accountType);

			// Act + Assert:
			AssertValidation(Failure_AccountLink_Remote_Account_Ineligible, cache, publicKey);
		}
	}

	TEST(TEST_CLASS, FailsWhenAccountExistsAndHasTypeUnlinked) {
		AssertFailsWhenAccountExistsWithType(state::AccountType::Unlinked);
	}

	TEST(TEST_CLASS, FailsWhenAccountExistsAndHasTypeMain) {
		AssertFailsWhenAccountExistsWithType(state::AccountType::Main);
	}

	TEST(TEST_CLASS, FailsWhenAccountExistsAndHasTypeRemote) {
		AssertFailsWhenAccountExistsWithType(state::AccountType::Remote);
	}

	TEST(TEST_CLASS, SucceedsWhenAccountDoesNotExist) {
		// Arrange:
		auto publicKey = test::GenerateRandomByteArray<Key>();
		auto cache = test::CoreSystemCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());

		// Act + Assert:
		AssertValidation(ValidationResult::Success, cache, publicKey);
	}

	TEST(TEST_CLASS, SucceedsWhenAccountExistsAndHasTypeRemoteUnlinked) {
		// Arrange:
		auto publicKey = test::GenerateRandomByteArray<Key>();
		auto cache = CreateCache(publicKey, state::AccountType::Remote_Unlinked);

		// Act + Assert:
		AssertValidation(ValidationResult::Success, cache, publicKey);
	}
}}
