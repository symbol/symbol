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

#define TEST_CLASS RemoteSenderValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(RemoteSender,)

	namespace {
		void AddAccount(cache::CatapultCache& cache, const Key& accountKey, state::AccountType accountType) {
			auto cacheDelta = cache.createDelta();
			auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();

			accountStateCacheDelta.addAccount(accountKey, Height(1));
			auto accountStateIter = accountStateCacheDelta.find(accountKey);
			auto& accountState = accountStateIter.get();
			accountState.AccountType = accountType;

			cache.commit(Height(1));
		}

		void AssertValidation(
				ValidationResult expectedResult,
				const Key& accountKey,
				state::AccountType accountType,
				const Key& notificationKey) {
			// Arrange:
			auto cache = test::CoreSystemCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			AddAccount(cache, accountKey, accountType);

			auto pValidator = CreateRemoteSenderValidator();
			auto entityType = static_cast<model::EntityType>(0x4201);
			auto notification = model::TransactionNotification(notificationKey, Hash256(), entityType, Timestamp());

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, FailureWhenAccountIsRemoteAndSigner) {
		// Arrange:
		auto accountKey = test::GenerateRandomByteArray<Key>();

		// Assert:
		AssertValidation(Failure_AccountLink_Remote_Account_Signer_Not_Allowed, accountKey, state::AccountType::Remote, accountKey);
	}

	TEST(TEST_CLASS, SuccessWhenSignerIsUnknown) {
		// Arrange:
		auto accountKey = test::GenerateRandomByteArray<Key>();
		auto notificationKey = test::GenerateRandomByteArray<Key>();

		// Assert:
		AssertValidation(ValidationResult::Success, accountKey, state::AccountType::Remote, notificationKey);
	}

	TEST(TEST_CLASS, SuccessWhenAccountIsMainAndSigner) {
		// Arrange:
		auto accountKey = test::GenerateRandomByteArray<Key>();

		// Assert:
		AssertValidation(ValidationResult::Success, accountKey, state::AccountType::Main, accountKey);
	}

	TEST(TEST_CLASS, SuccessWhenAccountIsUnlinkedAndSigner) {
		// Arrange:
		auto accountKey = test::GenerateRandomByteArray<Key>();

		// Assert:
		AssertValidation(ValidationResult::Success, accountKey, state::AccountType::Unlinked, accountKey);
	}
}}
