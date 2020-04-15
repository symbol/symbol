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

#define TEST_CLASS AccountLinkAvailabilityValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(AccountLinkAvailability,)

	// region test utils

	namespace {
		void AddLink(cache::CatapultCache& cache, const Key& mainAccountKey, const Key& linkedAccountKey, state::AccountType accountType) {
			auto cacheDelta = cache.createDelta();
			auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();

			accountStateCacheDelta.addAccount(mainAccountKey, Height(1));
			auto mainAccountStateIter = accountStateCacheDelta.find(mainAccountKey);
			auto& mainAccountState = mainAccountStateIter.get();

			mainAccountState.SupplementalAccountKeys.set(state::AccountKeyType::Linked, linkedAccountKey);
			mainAccountState.AccountType = accountType;

			cache.commit(Height(1));
		}

		void AssertValidation(ValidationResult expectedResult, state::AccountType accountType, model::LinkAction linkAction) {
			// Arrange:
			auto mainAccountKey = test::GenerateRandomByteArray<Key>();
			auto remoteAccountKey = test::GenerateRandomByteArray<Key>();

			auto cache = test::CoreSystemCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			AddLink(cache, mainAccountKey, remoteAccountKey, accountType);

			auto pValidator = CreateAccountLinkAvailabilityValidator();
			auto notification = model::RemoteAccountLinkNotification(mainAccountKey, remoteAccountKey, linkAction);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}
	}

	// endregion

	// region link - main account key type validation

	namespace {
		void AssertLinkValidationForAccountWithType(ValidationResult expectedResult, state::AccountType accountType) {
			// Assert:
			AssertValidation(expectedResult, accountType, model::LinkAction::Link);
		}
	}

	TEST(TEST_CLASS, LinkFailsWhenExistingAccountHasMainLink) {
		AssertLinkValidationForAccountWithType(Failure_AccountLink_Link_Already_Exists, state::AccountType::Main);
	}

	TEST(TEST_CLASS, LinkFailsWhenExistingAccountHasRemoteLink) {
		AssertLinkValidationForAccountWithType(Failure_AccountLink_Link_Already_Exists, state::AccountType::Remote);
	}

	TEST(TEST_CLASS, LinkSucceedsWhenExistingAccountHasNoLink) {
		AssertLinkValidationForAccountWithType(ValidationResult::Success, state::AccountType::Unlinked);
	}

	// endregion

	// region unlink - main account key type validation

	namespace {
		void AssertUnlinkValidationForAccountWithType(ValidationResult expectedResult, state::AccountType accountType) {
			// Assert:
			AssertValidation(expectedResult, accountType, model::LinkAction::Unlink);
		}
	}

	TEST(TEST_CLASS, UnlinkSucceedsWhenExistingAccountHasMainLink) {
		AssertUnlinkValidationForAccountWithType(ValidationResult::Success, state::AccountType::Main);
	}

	TEST(TEST_CLASS, UnlinkFailsWhenExistingAccountHasRemoteLink) {
		AssertUnlinkValidationForAccountWithType(Failure_AccountLink_Unknown_Link, state::AccountType::Remote);
	}

	TEST(TEST_CLASS, UnlinkFailsWhenExistingAccountHasNoLink) {
		AssertUnlinkValidationForAccountWithType(Failure_AccountLink_Unknown_Link, state::AccountType::Unlinked);
	}

	// endregion

	// region unlink - data consistency

	TEST(TEST_CLASS, UnlinkFailsWhenExistingAccountHasMainLinkButNotificationDataIsInconsistentWithState) {
		// Arrange:
		auto mainAccountKey = test::GenerateRandomByteArray<Key>();
		auto remoteAccountKey = test::GenerateRandomByteArray<Key>();

		auto cache = test::CoreSystemCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
		AddLink(cache, mainAccountKey, remoteAccountKey, state::AccountType::Main);

		// - the notification remote account key does not match the state remote account key
		auto pValidator = CreateAccountLinkAvailabilityValidator();
		auto notificationRemoteKey = test::GenerateRandomByteArray<Key>();
		auto unlinkAction = model::LinkAction::Unlink;
		auto notification = model::RemoteAccountLinkNotification(mainAccountKey, notificationRemoteKey, unlinkAction);

		// Act:
		auto result = test::ValidateNotification(*pValidator, notification, cache);

		// Assert:
		EXPECT_EQ(Failure_AccountLink_Inconsistent_Unlink_Data, result);
	}

	// endregion
}}
