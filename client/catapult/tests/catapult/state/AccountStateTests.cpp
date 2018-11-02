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
}}
