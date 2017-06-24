#include "AccountStateTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	void RandomFillAccountData(uint64_t seed, state::AccountState& state, size_t numMosaics) {
		for (auto i = 0u; i < numMosaics; ++i)
			state.Balances.credit(MosaicId(10 + i), Amount(seed * 1000 + i + 1));

		for (auto i = 0u; i < 3u; ++i) {
			auto importance = Importance(seed * (777 + i));
			auto importanceHeight = model::ConvertToImportanceHeight(Height(seed * 7 + (i + 1) * 23), 13u);
			state.ImportanceInfo.set(importance, importanceHeight);
		}
	}

	void AssertEqual(const state::AccountState& expected, const state::AccountState& actual) {
		// Assert:
		EXPECT_EQ(expected.Address, actual.Address);
		EXPECT_EQ(expected.AddressHeight, actual.AddressHeight);
		EXPECT_EQ(expected.PublicKey, actual.PublicKey);
		EXPECT_EQ(expected.PublicKeyHeight, actual.PublicKeyHeight);

		auto expectedIter = expected.ImportanceInfo.begin();
		auto actualIter = actual.ImportanceInfo.begin();
		for (auto i = 0u; i < Importance_History_Size; ++i, ++expectedIter, ++actualIter) {
			const auto message = "importance at " + std::to_string(i);
			EXPECT_EQ(expectedIter->Importance, actualIter->Importance) << message;
			EXPECT_EQ(expectedIter->Height, actualIter->Height) << message;
		}

		EXPECT_EQ(expected.ImportanceInfo.end(), expectedIter);
		EXPECT_EQ(actual.ImportanceInfo.end(), actualIter);

		EXPECT_EQ(expected.Balances.size(), actual.Balances.size());
		for (const auto& pair : expected.Balances)
			EXPECT_EQ(pair.second, actual.Balances.get(pair.first)) << "for mosaic " << pair.first;
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
