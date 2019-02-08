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
#include "src/model/AccountLinkTransaction.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/Address.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/plugins/ValidatorTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace validators {

#define TEST_CLASS RemoteInteractionValidatorTests

	DEFINE_COMMON_VALIDATOR_TESTS(RemoteInteraction,)

	namespace {
		template<typename TKey>
		void AddAccount(cache::CatapultCache& cache, const TKey& accountKey, state::AccountType accountType) {
			auto cacheDelta = cache.createDelta();
			auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();

			accountStateCacheDelta.addAccount(accountKey, Height(1));
			auto accountStateIter = accountStateCacheDelta.find(accountKey);
			auto& accountState = accountStateIter.get();
			accountState.AccountType = accountType;

			cache.commit(Height(1));
		}

		template<typename TKey>
		void AssertValidation(
				ValidationResult expectedResult,
				const Key& accountKey,
				state::AccountType accountType,
				std::vector<TKey> additionalCacheKeys,
				model::EntityType transactionType,
				const model::UnresolvedAddressSet& participantsByAddress,
				const utils::KeySet& participantsByKey) {
			// Arrange:
			auto cache = test::CoreSystemCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			AddAccount(cache, accountKey, accountType);
			for (const auto key : additionalCacheKeys)
				AddAccount(cache, key, state::AccountType::Main);

			auto pValidator = CreateRemoteInteractionValidator();
			auto notification = model::AddressInteractionNotification(Key(), transactionType, participantsByAddress, participantsByKey);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

		void AssertValidation(
				ValidationResult expectedResult,
				const Key& accountKey,
				state::AccountType accountType,
				model::EntityType transactionType,
				const model::UnresolvedAddressSet& participantsByAddress,
				const utils::KeySet& participantsByKey) {
			AssertValidation<Key>(expectedResult, accountKey, accountType, {}, transactionType, participantsByAddress, participantsByKey);
		}
	}

	TEST(TEST_CLASS, FailureWhenAccountIsRemoteAndContainedInParticipantsByAddress_SingleParticipant) {
		// Arrange:
		auto accountKey = test::GenerateRandomData<Key_Size>();
		auto accountAddress = test::UnresolveXor(model::PublicKeyToAddress(accountKey, model::NetworkIdentifier::Zero));
		constexpr auto Failure = Failure_AccountLink_Remote_Account_Participant_Not_Allowed;

		// Assert:
		AssertValidation(Failure, accountKey, state::AccountType::Remote, static_cast<model::EntityType>(0x4123), { accountAddress }, {});
	}

	TEST(TEST_CLASS, FailureWhenAccountIsRemoteAndContainedInParticipantsByKey_SingleParticipant) {
		// Arrange:
		auto accountKey = test::GenerateRandomData<Key_Size>();
		constexpr auto Failure = Failure_AccountLink_Remote_Account_Participant_Not_Allowed;

		// Assert:
		AssertValidation(Failure, accountKey, state::AccountType::Remote, static_cast<model::EntityType>(0x4123), {}, { accountKey });
	}

	TEST(TEST_CLASS, FailureWhenAccountIsRemoteAndContainedInParticipantsByAddress_MultipleParticipants) {
		// Arrange:
		auto accountKey = test::GenerateRandomData<Key_Size>();
		auto accountAddress = test::UnresolveXor(model::PublicKeyToAddress(accountKey, model::NetworkIdentifier::Zero));
		auto additionalParticipants = test::GenerateRandomDataVector<Address>(2);
		constexpr auto Failure = Failure_AccountLink_Remote_Account_Participant_Not_Allowed;

		// Assert:
		AssertValidation(
				Failure,
				accountKey,
				state::AccountType::Remote,
				additionalParticipants,
				static_cast<model::EntityType>(0x4123),
				{ test::UnresolveXor(additionalParticipants[0]), accountAddress, test::UnresolveXor(additionalParticipants[1]) },
				{});
	}

	TEST(TEST_CLASS, FailureWhenAccountIsRemoteAndContainedInParticipantsByKey_MultipleParticipants) {
		// Arrange:
		auto accountKey = test::GenerateRandomData<Key_Size>();
		auto additionalParticipants = test::GenerateRandomDataVector<Key>(2);
		constexpr auto Failure = Failure_AccountLink_Remote_Account_Participant_Not_Allowed;

		// Assert:
		AssertValidation(
				Failure,
				accountKey,
				state::AccountType::Remote,
				additionalParticipants,
				static_cast<model::EntityType>(0x4123),
				{},
				{ additionalParticipants[0], accountKey, additionalParticipants[1] });
	}

	TEST(TEST_CLASS, SuccessWhenAccountIsRemoteAndTransactionHasTypeAccountLink) {
		// Arrange:
		auto accountKey = test::GenerateRandomData<Key_Size>();
		auto accountAddress = test::UnresolveXor(model::PublicKeyToAddress(accountKey, model::NetworkIdentifier::Zero));
		constexpr auto transactionType = model::AccountLinkTransaction::Entity_Type;
		constexpr auto Success = ValidationResult::Success;

		// Assert:
		AssertValidation(Success, accountKey, state::AccountType::Remote, transactionType, { accountAddress }, { accountKey });
	}

	TEST(TEST_CLASS, SuccessWhenParticipantIsUnknown) {
		// Arrange:
		auto accountKey = test::GenerateRandomData<Key_Size>();
		auto participantAddress = test::GenerateRandomUnresolvedAddress();
		auto participantKey = test::GenerateRandomData<Key_Size>();
		auto transactionType = static_cast<model::EntityType>(0x4123);
		constexpr auto Success = ValidationResult::Success;

		// Assert:
		AssertValidation(Success, accountKey, state::AccountType::Remote, transactionType, { participantAddress }, { participantKey });
	}

	TEST(TEST_CLASS, SuccessWhenAccountIsNotRemote) {
		// Arrange:
		auto accountKey = test::GenerateRandomData<Key_Size>();
		auto accountAddress = test::UnresolveXor(model::PublicKeyToAddress(accountKey, model::NetworkIdentifier::Zero));
		auto transactionType = static_cast<model::EntityType>(0x4123);
		constexpr auto Success = ValidationResult::Success;

		// Assert:
		AssertValidation(Success, accountKey, state::AccountType::Main, transactionType, { accountAddress }, { accountKey });
		AssertValidation(Success, accountKey, state::AccountType::Unlinked, transactionType, { accountAddress }, { accountKey });
	}
}}
