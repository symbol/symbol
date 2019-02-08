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
		EXPECT_EQ(Key(), state.LinkedAccountKey);

		EXPECT_EQ(0u, state.Balances.size());

		for (const auto& pair : state.ImportanceInfo) {
			EXPECT_EQ(Importance(0), pair.Importance);
			EXPECT_EQ(model::ImportanceHeight(0), pair.Height);
		}
	}

	// endregion

	// region IsRemote

	TEST(TEST_CLASS, IsRemoteReturnsTrueForRemoteAccountTypes) {
		// Assert:
		EXPECT_TRUE(IsRemote(AccountType::Remote));
		EXPECT_TRUE(IsRemote(AccountType::Remote_Unlinked));
	}

	TEST(TEST_CLASS, IsRemoteReturnsFalseForNonRemoteAccountTypes) {
		// Assert:
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

			remoteAccountState.LinkedAccountKey = mainAccountState.PublicKey;
			mainAccountState.LinkedAccountKey = remoteAccountState.PublicKey;

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

	TEST(TEST_CLASS, RequireLinkedRemoteAndMainAccounts_ThrowsWhenRemoteAccountStateHasWrongLinkedAccountKey) {
		// Arrange:
		PrepareRequireLinkedRemoteAndMainAccountsTest([](auto& remoteAccountState, const auto& mainAccountState) {
			remoteAccountState.LinkedAccountKey[0] ^= 0xFF;

			// Act + Assert:
			EXPECT_THROW(RequireLinkedRemoteAndMainAccounts(remoteAccountState, mainAccountState), catapult_runtime_error);
		});
	}

	TEST(TEST_CLASS, RequireLinkedRemoteAndMainAccounts_ThrowsWhenMainAccountStateHasWrongLinkedAccountKey) {
		// Arrange:
		PrepareRequireLinkedRemoteAndMainAccountsTest([](const auto& remoteAccountState, auto& mainAccountState) {
			mainAccountState.LinkedAccountKey[0] ^= 0xFF;

			// Act + Assert:
			EXPECT_THROW(RequireLinkedRemoteAndMainAccounts(remoteAccountState, mainAccountState), catapult_runtime_error);
		});
	}

	// endregion
}}
