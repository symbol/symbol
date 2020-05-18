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
		void AddAccount(cache::CatapultCache& cache, const Key& accountPublicKey, state::AccountType accountType) {
			auto cacheDelta = cache.createDelta();
			auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();

			accountStateCacheDelta.addAccount(accountPublicKey, Height(1));
			auto accountStateIter = accountStateCacheDelta.find(accountPublicKey);
			auto& accountState = accountStateIter.get();
			accountState.AccountType = accountType;

			cache.commit(Height(1));
		}

		void AssertValidation(
				ValidationResult expectedResult,
				const Key& accountPublicKey,
				state::AccountType accountType,
				const Key& notificationPublicKey) {
			// Arrange:
			auto cache = test::CoreSystemCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			AddAccount(cache, accountPublicKey, accountType);

			auto pValidator = CreateRemoteSenderValidator();
			auto entityType = static_cast<model::EntityType>(0x4201);
			auto notification = model::TransactionNotification(notificationPublicKey, Hash256(), entityType, Timestamp());

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	TEST(TEST_CLASS, FailureWhenAccountIsRemoteAndSender) {
		// Arrange:
		auto accountPublicKey = test::GenerateRandomByteArray<Key>();

		// Assert:
		constexpr auto Failure_Result = Failure_AccountLink_Remote_Account_Signer_Prohibited;
		AssertValidation(Failure_Result, accountPublicKey, state::AccountType::Remote, accountPublicKey);
	}

	TEST(TEST_CLASS, SuccessWhenSenderIsUnknown) {
		// Arrange:
		auto accountPublicKey = test::GenerateRandomByteArray<Key>();
		auto notificationPublicKey = test::GenerateRandomByteArray<Key>();

		// Assert:
		AssertValidation(ValidationResult::Success, accountPublicKey, state::AccountType::Remote, notificationPublicKey);
	}

	TEST(TEST_CLASS, SuccessWhenAccountIsMainAndSender) {
		// Arrange:
		auto accountPublicKey = test::GenerateRandomByteArray<Key>();

		// Assert:
		AssertValidation(ValidationResult::Success, accountPublicKey, state::AccountType::Main, accountPublicKey);
	}

	TEST(TEST_CLASS, SuccessWhenAccountIsUnlinkedAndSender) {
		// Arrange:
		auto accountPublicKey = test::GenerateRandomByteArray<Key>();

		// Assert:
		AssertValidation(ValidationResult::Success, accountPublicKey, state::AccountType::Unlinked, accountPublicKey);
	}
}}
