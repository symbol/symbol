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

#include "AccountStateTestUtils.h"
#include "AddressTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	void RandomFillAccountData(uint64_t seed, state::AccountState& state, size_t numMosaics) {
		for (auto i = 0u; i < numMosaics; ++i)
			state.Balances.credit(MosaicId(10 + i + ((0 == i % 2) ? 100 : 0)), Amount(seed * 1000 + i + 1));

		for (auto i = 0u; i < 3; ++i) {
			auto importance = Importance(seed * (777 + i));
			auto importanceHeight = model::ConvertToImportanceHeight(Height(seed * 7 + (i + 1) * 23), 13);
			state.ImportanceInfo.set(importance, importanceHeight);
		}
	}

	void AssertEqual(const state::AccountState& expected, const state::AccountState& actual, const std::string& message) {
		// Assert:
		EXPECT_EQ(expected.Address, actual.Address) << message;
		EXPECT_EQ(expected.AddressHeight, actual.AddressHeight) << message;
		EXPECT_EQ(expected.PublicKey, actual.PublicKey) << message;
		EXPECT_EQ(expected.PublicKeyHeight, actual.PublicKeyHeight) << message;

		EXPECT_EQ(expected.AccountType, actual.AccountType) << message;
		EXPECT_EQ(expected.LinkedAccountKey, actual.LinkedAccountKey) << message;

		auto expectedIter = expected.ImportanceInfo.begin();
		auto actualIter = actual.ImportanceInfo.begin();
		for (auto i = 0u; i < Importance_History_Size; ++i, ++expectedIter, ++actualIter) {
			const auto importanceMessage = message + ": importance at " + std::to_string(i);
			EXPECT_EQ(expectedIter->Importance, actualIter->Importance) << importanceMessage;
			EXPECT_EQ(expectedIter->Height, actualIter->Height) << importanceMessage;
		}

		EXPECT_EQ(expected.ImportanceInfo.end(), expectedIter) << message;
		EXPECT_EQ(actual.ImportanceInfo.end(), actualIter) << message;

		EXPECT_EQ(expected.Balances.size(), actual.Balances.size()) << message;
		for (const auto& pair : expected.Balances)
			EXPECT_EQ(pair.second, actual.Balances.get(pair.first)) << message << ": for mosaic " << pair.first;
	}

	std::shared_ptr<state::AccountState> CreateAccountStateWithoutPublicKey(uint64_t height) {
		auto address = test::GenerateRandomAddress();
		auto pState = std::make_shared<state::AccountState>(address, Height(height));
		pState->Balances.credit(Xem_Id, Amount(1));
		pState->ImportanceInfo.set(Importance(123456), model::ImportanceHeight(height));
		return pState;
	}

	AccountStates CreateAccountStates(size_t count) {
		AccountStates accountStates;
		for (auto i = 1u; i <= count; ++i) {
			accountStates.push_back(test::CreateAccountStateWithoutPublicKey(1));
			accountStates.back()->PublicKey = { { static_cast<uint8_t>(i) } };
		}

		return accountStates;
	}
}}
