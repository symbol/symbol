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
#include "catapult/utils/Casting.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	void RandomFillAccountData(uint64_t seed, state::AccountState& accountState, size_t numMosaics) {
		for (auto i = 0u; i < numMosaics; ++i)
			accountState.Balances.credit(MosaicId(10 + i + ((0 == i % 2) ? 100 : 0)), Amount(seed * 1000 + i + 1));

		for (auto i = 0u; i < Importance_History_Size; ++i) {
			auto importance = Importance(seed * (777 + i));
			auto importanceHeight = model::ConvertToImportanceHeight(Height(seed * 7 + (i + 1) * 23), 13);
			accountState.ImportanceSnapshots.set(importance, importanceHeight);
		}

		for (auto i = 0u; i < Activity_Bucket_History_Size; ++i) {
			auto importanceHeight = model::ConvertToImportanceHeight(Height(seed * 6 + (i + 1) * 23), 13);
			accountState.ActivityBuckets.update(importanceHeight, [seed, i](auto& bucket) {
				bucket.TotalFeesPaid = Amount(seed * (i + 1));
				bucket.BeneficiaryCount = static_cast<uint32_t>(seed + i);
				bucket.RawScore = seed * (i + 1) * (i + 1);
			});
		}
	}

	namespace {
		void AssertEqual(const state::AccountKeys& expected, const state::AccountKeys& actual, const std::string& message) {
			EXPECT_EQ(expected.mask(), actual.mask()) << message;
			EXPECT_EQ(expected.linkedPublicKey().get(), actual.linkedPublicKey().get()) << message;
			EXPECT_EQ(expected.vrfPublicKey().get(), actual.vrfPublicKey().get()) << message;
			EXPECT_EQ(expected.votingPublicKey().get(), actual.votingPublicKey().get()) << message;
			EXPECT_EQ(expected.nodePublicKey().get(), actual.nodePublicKey().get()) << message;
		}

		void AssertEqual(
				const state::AccountImportanceSnapshots& expected,
				const state::AccountImportanceSnapshots& actual,
				const std::string& message) {
			auto expectedIter = expected.begin();
			auto actualIter = actual.begin();
			for (auto i = 0u; i < Importance_History_Size; ++i, ++expectedIter, ++actualIter) {
				EXPECT_EQ(expectedIter->Importance, actualIter->Importance) << message << " at " << i;
				EXPECT_EQ(expectedIter->Height, actualIter->Height) << message << " at " << i;
			}

			EXPECT_EQ(expected.end(), expectedIter) << message;
			EXPECT_EQ(actual.end(), actualIter) << message;
		}

		void AssertEqual(
				const state::AccountActivityBuckets& expected,
				const state::AccountActivityBuckets& actual,
				const std::string& message) {
			auto expectedIter = expected.begin();
			auto actualIter = actual.begin();
			for (auto i = 0u; i < Activity_Bucket_History_Size; ++i, ++expectedIter, ++actualIter) {
				EXPECT_EQ(expectedIter->StartHeight, actualIter->StartHeight) << message << " at " << i;
				EXPECT_EQ(expectedIter->TotalFeesPaid, actualIter->TotalFeesPaid) << message << " at " << i;
				EXPECT_EQ(expectedIter->BeneficiaryCount, actualIter->BeneficiaryCount) << message << " at " << i;
				EXPECT_EQ(expectedIter->RawScore, actualIter->RawScore) << message << " at " << i;
			}

			EXPECT_EQ(expected.end(), expectedIter) << message;
			EXPECT_EQ(actual.end(), actualIter) << message;
		}
	}

	void AssertEqual(const state::AccountState& expected, const state::AccountState& actual, const std::string& message) {
		// Assert:
		EXPECT_EQ(expected.Address, actual.Address) << message;
		EXPECT_EQ(expected.AddressHeight, actual.AddressHeight) << message;
		EXPECT_EQ(expected.PublicKey, actual.PublicKey) << message;
		EXPECT_EQ(expected.PublicKeyHeight, actual.PublicKeyHeight) << message;

		EXPECT_EQ(expected.AccountType, actual.AccountType) << message;

		AssertEqual(expected.SupplementalAccountKeys, actual.SupplementalAccountKeys, message + ": supplemental account keys");
		AssertEqual(expected.ImportanceSnapshots, actual.ImportanceSnapshots, message + ": importance snapshot");
		AssertEqual(expected.ActivityBuckets, actual.ActivityBuckets, message + ": activity bucket");

		EXPECT_EQ(expected.Balances.optimizedMosaicId(), actual.Balances.optimizedMosaicId()) << message;
		EXPECT_EQ(expected.Balances.size(), actual.Balances.size()) << message;
		for (const auto& pair : expected.Balances)
			EXPECT_EQ(pair.second, actual.Balances.get(pair.first)) << message << ": for mosaic " << pair.first;
	}

	std::shared_ptr<state::AccountState> CreateAccountStateWithoutPublicKey(uint64_t height) {
		auto address = test::GenerateRandomAddress();
		auto pState = std::make_shared<state::AccountState>(address, Height(height));
		pState->Balances.credit(MosaicId(1111), Amount(1));
		pState->ImportanceSnapshots.set(Importance(123456), model::ImportanceHeight(height));
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

	void SetRandomSupplementalAccountKeys(state::AccountState& accountState, state::AccountKeys::KeyType mask) {
		if (HasFlag(state::AccountKeys::KeyType::Linked, mask))
			accountState.SupplementalAccountKeys.linkedPublicKey().set(test::GenerateRandomByteArray<Key>());

		if (HasFlag(state::AccountKeys::KeyType::VRF, mask))
			accountState.SupplementalAccountKeys.vrfPublicKey().set(test::GenerateRandomByteArray<Key>());

		if (HasFlag(state::AccountKeys::KeyType::Voting, mask))
			accountState.SupplementalAccountKeys.votingPublicKey().set(test::GenerateRandomByteArray<VotingKey>());

		if (HasFlag(state::AccountKeys::KeyType::Node, mask))
			accountState.SupplementalAccountKeys.nodePublicKey().set(test::GenerateRandomByteArray<Key>());
	}

	void ForceSetLinkedPublicKey(state::AccountState& accountState, const Key& linkedPublicKey) {
		accountState.SupplementalAccountKeys.linkedPublicKey().unset();
		accountState.SupplementalAccountKeys.linkedPublicKey().set(linkedPublicKey);
	}
}}
