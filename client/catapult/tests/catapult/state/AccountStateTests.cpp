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

#include "catapult/state/AccountState.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

#define TEST_CLASS AccountStateTests

	// region constructor

	TEST(TEST_CLASS, AccountStateCtorInitializesProperFields) {
		// Arrange:
		auto address = test::GenerateRandomAddress();
		auto height = Height(1234);

		// Act:
		AccountState state(address, height);

		// Assert:
		EXPECT_EQ(address, state.Address);
		EXPECT_EQ(height, state.AddressHeight);

		EXPECT_EQ(Key(), state.PublicKey);
		EXPECT_EQ(Height(0), state.PublicKeyHeight);

		EXPECT_EQ(AccountType::Unlinked, state.AccountType);
		EXPECT_EQ(AccountPublicKeys::KeyType::Unset, state.SupplementalPublicKeys.mask());

		for (const auto& snapshot : state.ImportanceSnapshots) {
			EXPECT_EQ(Importance(0), snapshot.Importance);
			EXPECT_EQ(model::ImportanceHeight(0), snapshot.Height);
		}

		for (const auto& bucket : state.ActivityBuckets) {
			EXPECT_EQ(model::ImportanceHeight(0), bucket.StartHeight);
			EXPECT_EQ(Amount(0), bucket.TotalFeesPaid);
			EXPECT_EQ(0u, bucket.BeneficiaryCount);
			EXPECT_EQ(0u, bucket.RawScore);
		}

		EXPECT_EQ(0u, state.Balances.size());
	}

	// endregion

	// region IsRemote

	TEST(TEST_CLASS, IsRemoteReturnsTrueForRemoteAccountTypes) {
		EXPECT_TRUE(IsRemote(AccountType::Remote));
		EXPECT_TRUE(IsRemote(AccountType::Remote_Unlinked));
	}

	TEST(TEST_CLASS, IsRemoteReturnsFalseForNonRemoteAccountTypes) {
		EXPECT_FALSE(IsRemote(AccountType::Unlinked));
		EXPECT_FALSE(IsRemote(AccountType::Main));
		EXPECT_FALSE(IsRemote(static_cast<AccountType>(234)));
	}

	// endregion

	// region RequireLinkedRemoteAndMainAccounts

	namespace {
		template<typename TAction>
		void PrepareRequireLinkedRemoteAndMainAccountsTest(TAction action) {
			// Arrange: set up two linked accounts
			AccountState remoteAccountState(test::GenerateRandomAddress(), Height(1));
			test::FillWithRandomData(remoteAccountState.PublicKey);
			remoteAccountState.AccountType = AccountType::Remote;

			AccountState mainAccountState(test::GenerateRandomAddress(), Height(1));
			test::FillWithRandomData(mainAccountState.PublicKey);
			mainAccountState.AccountType = AccountType::Main;

			remoteAccountState.SupplementalPublicKeys.linked().set(mainAccountState.PublicKey);
			mainAccountState.SupplementalPublicKeys.linked().set(remoteAccountState.PublicKey);

			// Act + Assert:
			action(remoteAccountState, mainAccountState);
		}
	}

	TEST(TEST_CLASS, RequireLinkedRemoteAndMainAccounts_DoesNotThrowWhenLinksAreProper) {
		// Arrange:
		PrepareRequireLinkedRemoteAndMainAccountsTest([](const auto& remoteAccountState, const auto& mainAccountState) {
			// Act + Assert:
			EXPECT_NO_THROW(RequireLinkedRemoteAndMainAccounts(remoteAccountState, mainAccountState));
		});
	}

	TEST(TEST_CLASS, RequireLinkedRemoteAndMainAccounts_ThrowsWhenLinksAreReversed) {
		// Arrange:
		PrepareRequireLinkedRemoteAndMainAccountsTest([](const auto& remoteAccountState, const auto& mainAccountState) {
			// Act + Assert:
			EXPECT_THROW(RequireLinkedRemoteAndMainAccounts(mainAccountState, remoteAccountState), catapult_runtime_error);
		});
	}

	TEST(TEST_CLASS, RequireLinkedRemoteAndMainAccounts_ThrowsWhenRemoteAccountStateHasWrongAccountType) {
		// Arrange:
		PrepareRequireLinkedRemoteAndMainAccountsTest([](auto& remoteAccountState, const auto& mainAccountState) {
			remoteAccountState.AccountType = AccountType::Unlinked;

			// Act + Assert:
			EXPECT_THROW(RequireLinkedRemoteAndMainAccounts(remoteAccountState, mainAccountState), catapult_runtime_error);
		});
	}

	TEST(TEST_CLASS, RequireLinkedRemoteAndMainAccounts_ThrowsWhenMainAccountStateHasWrongAccountType) {
		// Arrange:
		PrepareRequireLinkedRemoteAndMainAccountsTest([](const auto& remoteAccountState, auto& mainAccountState) {
			mainAccountState.AccountType = AccountType::Unlinked;

			// Act + Assert:
			EXPECT_THROW(RequireLinkedRemoteAndMainAccounts(remoteAccountState, mainAccountState), catapult_runtime_error);
		});
	}

	namespace {
		void MutateLinkedPublicKey(AccountState& accountState) {
			auto& accountPublicKeys = accountState.SupplementalPublicKeys;

			auto linkedPublicKey = accountPublicKeys.linked().get();
			linkedPublicKey[0] ^= 0xFF;

			accountPublicKeys.linked().unset();
			accountPublicKeys.linked().set(linkedPublicKey);
		}
	}

	TEST(TEST_CLASS, RequireLinkedRemoteAndMainAccounts_ThrowsWhenRemoteAccountStateHasWrongLinkedPublicKey) {
		// Arrange:
		PrepareRequireLinkedRemoteAndMainAccountsTest([](auto& remoteAccountState, const auto& mainAccountState) {
			MutateLinkedPublicKey(remoteAccountState);

			// Act + Assert:
			EXPECT_THROW(RequireLinkedRemoteAndMainAccounts(remoteAccountState, mainAccountState), catapult_runtime_error);
		});
	}

	TEST(TEST_CLASS, RequireLinkedRemoteAndMainAccounts_ThrowsWhenMainAccountStateHasWrongLinkedPublicKey) {
		// Arrange:
		PrepareRequireLinkedRemoteAndMainAccountsTest([](const auto& remoteAccountState, auto& mainAccountState) {
			MutateLinkedPublicKey(mainAccountState);

			// Act + Assert:
			EXPECT_THROW(RequireLinkedRemoteAndMainAccounts(remoteAccountState, mainAccountState), catapult_runtime_error);
		});
	}

	// endregion

	// region ApplyFeeSurplus

	TEST(TEST_CLASS, ApplyFeeSurplus_AppliesFeeSurplusToAccountWithoutActivityBucket) {
		// Arrange:
		AccountState accountState(test::GenerateRandomAddress(), Height(1));
		accountState.Balances.credit(MosaicId(123), Amount(1000));

		// Act:
		ApplyFeeSurplus(accountState, { MosaicId(123), Amount(111) }, model::ImportanceHeight(100));

		// Assert: balance was credited but no new activity bucket was created
		EXPECT_EQ(Amount(1111), accountState.Balances.get(MosaicId(123)));

		auto activityBucket = accountState.ActivityBuckets.get(model::ImportanceHeight(100));
		EXPECT_EQ(model::ImportanceHeight(), activityBucket.StartHeight);
	}

	TEST(TEST_CLASS, ApplyFeeSurplus_AppliesFeeSurplusToAccountWithActivityBucket) {
		// Arrange:
		AccountState accountState(test::GenerateRandomAddress(), Height(1));
		accountState.Balances.credit(MosaicId(123), Amount(1000));
		accountState.ActivityBuckets.update(model::ImportanceHeight(100), [](auto& bucket) {
			bucket.TotalFeesPaid = Amount(333);
		});

		// Act:
		ApplyFeeSurplus(accountState, { MosaicId(123), Amount(111) }, model::ImportanceHeight(100));

		// Assert: balance was credited and total fees paid was reduced
		EXPECT_EQ(Amount(1111), accountState.Balances.get(MosaicId(123)));

		auto activityBucket = accountState.ActivityBuckets.get(model::ImportanceHeight(100));
		EXPECT_EQ(model::ImportanceHeight(100), activityBucket.StartHeight);
		EXPECT_EQ(Amount(222), activityBucket.TotalFeesPaid);
	}

	// endregion

	// region account key accessors

	TEST(TEST_CLASS, CanRetrieveSupplementalPublicKeysViaAccessorsWhenUnset) {
		// Arrange:
		AccountState accountState(test::GenerateRandomAddress(), Height(123));

		// Act + Assert:
		EXPECT_EQ(Key(), GetLinkedPublicKey(accountState));
		EXPECT_EQ(Key(), GetNodePublicKey(accountState));
		EXPECT_EQ(Key(), GetVrfPublicKey(accountState));
	}

	TEST(TEST_CLASS, CanRetrieveSupplementalPublicKeysViaAccessorsWhenSet) {
		// Arrange:
		auto linkedPublicKey = test::GenerateRandomByteArray<Key>();
		auto nodePublicKey = test::GenerateRandomByteArray<Key>();
		auto vrfPublicKey = test::GenerateRandomByteArray<Key>();

		AccountState accountState(test::GenerateRandomAddress(), Height(123));
		accountState.SupplementalPublicKeys.linked().set(linkedPublicKey);
		accountState.SupplementalPublicKeys.node().set(nodePublicKey);
		accountState.SupplementalPublicKeys.vrf().set(vrfPublicKey);

		// Act + Assert:
		EXPECT_EQ(linkedPublicKey, GetLinkedPublicKey(accountState));
		EXPECT_EQ(nodePublicKey, GetNodePublicKey(accountState));
		EXPECT_EQ(vrfPublicKey, GetVrfPublicKey(accountState));
	}

	// endregion
}}
