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
#include "src/model/AccountKeyLinkTransaction.h"
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
		void AddAccount(cache::CatapultCache& cache, const Address& address, state::AccountType accountType) {
			auto cacheDelta = cache.createDelta();
			auto& accountStateCacheDelta = cacheDelta.sub<cache::AccountStateCache>();

			accountStateCacheDelta.addAccount(address, Height(1));
			auto accountStateIter = accountStateCacheDelta.find(address);
			auto& accountState = accountStateIter.get();
			accountState.AccountType = accountType;

			cache.commit(Height(1));
		}

		void AssertValidation(
				ValidationResult expectedResult,
				const Address& address,
				state::AccountType accountType,
				std::vector<Address> additionalCacheAddresses,
				model::EntityType transactionType,
				const model::UnresolvedAddressSet& participantsByAddress) {
			// Arrange:
			auto cache = test::CoreSystemCacheFactory::Create(model::BlockChainConfiguration::Uninitialized());
			AddAccount(cache, address, accountType);
			for (const auto& additionalAddress : additionalCacheAddresses)
				AddAccount(cache, additionalAddress, state::AccountType::Main);

			auto pValidator = CreateRemoteInteractionValidator();
			auto notification = model::AddressInteractionNotification(Address(), transactionType, participantsByAddress);

			// Act:
			auto result = test::ValidateNotification(*pValidator, notification, cache);

			// Assert:
			EXPECT_EQ(expectedResult, result);
		}

		void AssertValidation(
				ValidationResult expectedResult,
				const Address& address,
				state::AccountType accountType,
				model::EntityType transactionType,
				const model::UnresolvedAddressSet& participantsByAddress) {
			AssertValidation(expectedResult, address, accountType, {}, transactionType, participantsByAddress);
		}
	}

	TEST(TEST_CLASS, FailureWhenAccountIsRemoteAndContainedInParticipantsByAddress_SingleParticipant) {
		// Arrange:
		constexpr auto Failure = Failure_AccountLink_Remote_Account_Participant_Prohibited;

		auto address = test::GenerateRandomByteArray<Address>();

		// Assert:
		AssertValidation(Failure, address, state::AccountType::Remote, static_cast<model::EntityType>(0x4123), {
			test::UnresolveXor(address)
		});
	}

	TEST(TEST_CLASS, FailureWhenAccountIsRemoteAndContainedInParticipantsByAddress_MultipleParticipants) {
		// Arrange:
		constexpr auto Failure = Failure_AccountLink_Remote_Account_Participant_Prohibited;

		auto address = test::GenerateRandomByteArray<Address>();
		auto additionalParticipants = test::GenerateRandomDataVector<Address>(2);

		// Assert:
		AssertValidation(Failure, address, state::AccountType::Remote, additionalParticipants, static_cast<model::EntityType>(0x4123), {
			test::UnresolveXor(additionalParticipants[0]),
			test::UnresolveXor(address),
			test::UnresolveXor(additionalParticipants[1])
		});
	}

	TEST(TEST_CLASS, SuccessWhenAccountIsRemoteAndTransactionHasTypeAccountLink) {
		// Arrange:
		constexpr auto Transaction_Type = model::AccountKeyLinkTransaction::Entity_Type;
		constexpr auto Success = ValidationResult::Success;

		auto address = test::GenerateRandomByteArray<Address>();

		// Assert:
		AssertValidation(Success, address, state::AccountType::Remote, Transaction_Type, { test::UnresolveXor(address) });
	}

	TEST(TEST_CLASS, SuccessWhenParticipantIsUnknown) {
		// Arrange:
		constexpr auto Transaction_Type = static_cast<model::EntityType>(0x4123);
		constexpr auto Success = ValidationResult::Success;

		auto address = test::GenerateRandomByteArray<Address>();
		auto participantAddress = test::GenerateRandomByteArray<Address>();

		// Assert:
		AssertValidation(Success, address, state::AccountType::Remote, Transaction_Type, { test::UnresolveXor(participantAddress) });
	}

	TEST(TEST_CLASS, SuccessWhenAccountIsNotRemote) {
		// Arrange:
		constexpr auto Transaction_Type = static_cast<model::EntityType>(0x4123);
		constexpr auto Success = ValidationResult::Success;

		auto address = test::GenerateRandomByteArray<Address>();

		// Assert:
		AssertValidation(Success, address, state::AccountType::Main, Transaction_Type, { test::UnresolveXor(address) });
		AssertValidation(Success, address, state::AccountType::Unlinked, Transaction_Type, { test::UnresolveXor(address) });
	}
}}
