#include "catapult/state/AccountState.h"
#include "catapult/model/AccountInfo.h"
#include "tests/test/core/AccountStateTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"

namespace catapult { namespace state {

	TEST(AccountStateTests, AccountStateCtorInitializesProperFields) {
		// Arrange:
		auto address = test::GenerateRandomAddress();
		auto height = Height(1234);

		// Act:
		AccountState state(address, height);

		// Assert:
		EXPECT_EQ(address, state.Address);
		EXPECT_EQ(height, state.AddressHeight);
		EXPECT_EQ(Height(0), state.PublicKeyHeight);
		EXPECT_EQ(0u, state.Balances.size());

		for (const auto& pair : state.ImportanceInfo) {
			EXPECT_EQ(Importance(0), pair.Importance);
			EXPECT_EQ(model::ImportanceHeight(0), pair.Height);
		}
	}

	// region AccountState -> AccountInfo

	namespace {
		AccountState CreateAccountStateWithZeroMosaics(size_t numImportances = Importance_History_Size) {
			AccountState accountState(test::GenerateRandomData<Address_Decoded_Size>(), Height(123));
			accountState.PublicKey = test::GenerateRandomData<Key_Size>();
			accountState.PublicKeyHeight = Height(456);
			for (auto i = 1u; i <= numImportances; ++i)
				accountState.ImportanceInfo.set(Importance(i * i), model::ImportanceHeight(i));

			return accountState;
		}

		void AssertNonMosaicPart(const model::AccountInfo& info, const AccountState& accountState, uint32_t expectedNumMosaics) {
			// Assert:
			EXPECT_EQ(sizeof(model::AccountInfo) + expectedNumMosaics * sizeof(model::Mosaic), info.Size);
			EXPECT_EQ(accountState.Address, info.Address);
			EXPECT_EQ(accountState.AddressHeight, info.AddressHeight);
			EXPECT_EQ(accountState.PublicKey, info.PublicKey);
			EXPECT_EQ(accountState.PublicKeyHeight, info.PublicKeyHeight);
			EXPECT_EQ(expectedNumMosaics, info.NumMosaics);

			auto i = 0u;
			for (const auto& pair : accountState.ImportanceInfo) {
				const auto message = "importance at " + std::to_string(i);
				EXPECT_EQ(pair.Importance, info.Importances[i]) << message;
				EXPECT_EQ(pair.Height, info.ImportanceHeights[i]) << message;
				++i;
			}
		}

		void AssertMosaic(const model::Mosaic& mosaic, MosaicId expectedId, Amount expectedAmount) {
			// Assert:
			EXPECT_EQ(expectedId, mosaic.MosaicId);
			EXPECT_EQ(expectedAmount, mosaic.Amount);
		}

		void AssertToAccountInfoInitializesAllAccountInfoFieldsZeroMosaics(size_t numImportances) {
			// Arrange:
			auto accountState = CreateAccountStateWithZeroMosaics(numImportances);

			// Act:
			auto pAccountInfo = accountState.toAccountInfo();

			// Assert:
			AssertNonMosaicPart(*pAccountInfo, accountState, 0);
			EXPECT_FALSE(!!pAccountInfo->MosaicsPtr());
		}
	}

	TEST(AccountStateTests, ToAccountInfoInitializesAllAccountInfoFields_ZeroMosaics) {
		// Assert:
		AssertToAccountInfoInitializesAllAccountInfoFieldsZeroMosaics(Importance_History_Size);
	}

	TEST(AccountStateTests, ToAccountInfoInitializesAllAccountInfoFields_SingleMosaic) {
		// Arrange:
		auto accountState = CreateAccountStateWithZeroMosaics();
		accountState.Balances.credit(Xem_Id, Amount(13579));

		// Act:
		auto pAccountInfo = accountState.toAccountInfo();

		// Assert:
		AssertNonMosaicPart(*pAccountInfo, accountState, 1);

		auto pMosaic = pAccountInfo->MosaicsPtr();
		ASSERT_TRUE(!!pMosaic);
		AssertMosaic(*pMosaic, Xem_Id, Amount(13579));
	}

	TEST(AccountStateTests, ToAccountInfoInitializesAllAccountInfoFields_MultipleMosaics) {
		// Arrange:
		auto accountState = CreateAccountStateWithZeroMosaics();
		accountState.Balances.credit(MosaicId(123), Amount(111));
		accountState.Balances.credit(Xem_Id, Amount(13579));
		accountState.Balances.credit(MosaicId(987), Amount(222));

		// Act:
		auto pAccountInfo = accountState.toAccountInfo();

		// Assert:
		AssertNonMosaicPart(*pAccountInfo, accountState, 3);

		auto pMosaic = pAccountInfo->MosaicsPtr();
		ASSERT_TRUE(!!pMosaic);

		std::map<MosaicId, model::Mosaic> mosaics;
		for (auto i = 0u; i < 3u; ++i) {
			mosaics.emplace(pMosaic->MosaicId, *pMosaic);
			++pMosaic;
		}

		AssertMosaic(mosaics[MosaicId(123)], MosaicId(123), Amount(111));
		AssertMosaic(mosaics[Xem_Id], Xem_Id, Amount(13579));
		AssertMosaic(mosaics[MosaicId(987)], MosaicId(987), Amount(222));
	}

	TEST(AccountStateTests, ToAccountInfoInitializesAllAccountInfoFields_NoImportanceHistory) {
		// Assert:
		AssertToAccountInfoInitializesAllAccountInfoFieldsZeroMosaics(0);
	}

	TEST(AccountStateTests, ToAccountInfoInitializesAllAccountInfoFields_PartialImportanceHistory) {
		// Assert:
		AssertToAccountInfoInitializesAllAccountInfoFieldsZeroMosaics(Importance_History_Size - 1);
	}

	// endregion

	// region AccountState <- AccountInfo

	namespace {
		void AssertCanCreateAccountStateFromAccountInfo(
				const std::vector<model::Mosaic>& mosaics,
				size_t numImportances = Importance_History_Size) {
			// Arrange:
			auto originalAccountState = CreateAccountStateWithZeroMosaics(numImportances);
			for (const auto& mosaic : mosaics)
				originalAccountState.Balances.credit(mosaic.MosaicId, mosaic.Amount);

			auto pAccountInfo = originalAccountState.toAccountInfo();

			// Act:
			auto accountState = AccountState(*pAccountInfo);

			// Assert:
			EXPECT_EQ(mosaics.size(), accountState.Balances.size());
			test::AssertEqual(originalAccountState, accountState);
		}
	}

	TEST(AccountStateTests, CanCreateAccountStateFromAccountInfo_ZeroMosaics) {
		// Assert:
		AssertCanCreateAccountStateFromAccountInfo({});
	}

	TEST(AccountStateTests, CanCreateAccountStateFromAccountInfo_SingleMosaic) {
		// Assert:
		AssertCanCreateAccountStateFromAccountInfo({ { Xem_Id, Amount(13579) } });
	}

	TEST(AccountStateTests, CanCreateAccountStateFromAccountInfo_MultipleMosaics) {
		// Assert:
		AssertCanCreateAccountStateFromAccountInfo({
			{ MosaicId(123), Amount(111) },
			{ Xem_Id, Amount(13579) },
			{ MosaicId(987), Amount(222) }
		});
	}

	TEST(AccountStateTests, CanCreateAccountStateFromAccountInfo_NoImportanceHistory) {
		// Assert:
		AssertCanCreateAccountStateFromAccountInfo({}, 0);
	}

	TEST(AccountStateTests, CanCreateAccountStateFromAccountInfo_PartialImportanceHistory) {
		// Assert:
		AssertCanCreateAccountStateFromAccountInfo({}, Importance_History_Size - 1);
	}

	// endregion
}}
