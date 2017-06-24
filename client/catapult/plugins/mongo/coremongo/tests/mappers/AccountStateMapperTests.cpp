#include "src/mappers/AccountStateMapper.h"
#include "catapult/state/AccountState.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/mongo/MapperTestUtils.h"
#include "tests/TestHarness.h"
#include <mongocxx/client.hpp>

namespace catapult { namespace mongo { namespace mappers {

	// region ToDbModel

	namespace {
		void AssertEqualAccountStateMetadata() {
			// nothing to compare
		}

		auto CreateAccountState(Height publicKeyHeight, std::initializer_list<model::Mosaic> mosaics) {
			state::AccountState state(test::GenerateRandomAddress(), Height(123));
			if (Height(0) != publicKeyHeight) {
				state.PublicKeyHeight = publicKeyHeight;
				state.PublicKey = test::GenerateRandomData<Key_Size>();
			}

			auto numImportances = 1u + test::Random() % 3;
			for (auto i = 0u; i < numImportances; ++i)
				state.ImportanceInfo.set(Importance(234 + i * 100), model::ImportanceHeight(345 + i * 10));

			for (const auto& mosaic : mosaics)
				state.Balances.credit(mosaic.MosaicId, mosaic.Amount);

			return state;
		}

		void AssertCanMapAccountState(Height publicKeyHeight, std::initializer_list<model::Mosaic> mosaics) {
			// Arrange:
			auto state = CreateAccountState(publicKeyHeight, mosaics);

			// Act:
			auto dbAccount = ToDbModel(state);

			// Assert:
			auto view = dbAccount.view();
			EXPECT_EQ(2u, test::GetFieldCount(view));
			AssertEqualAccountStateMetadata();

			auto account = view["account"].get_document().view();
			EXPECT_EQ(6u, test::GetFieldCount(account));
			test::AssertEqualAccountState(state, account);
		}
	}

	TEST(AccountStateMapperTests, CanMapAccountStateWithNeitherPublicKeyNorMosaics) {
		// Assert:
		AssertCanMapAccountState(Height(0), {});
	}

	TEST(AccountStateMapperTests, CanMapAccountStateWithPublicKeyButWithoutMosaics) {
		// Assert:
		AssertCanMapAccountState(Height(456), {});
	}

	TEST(AccountStateMapperTests, CanMapAccountStateWithoutPublicKeyButWithSingleMosaic) {
		// Assert:
		AssertCanMapAccountState(Height(0), { { Xem_Id, Amount(234) } });
	}

	TEST(AccountStateMapperTests, CanMapAccountStateWithoutPublicKeyButWithMultipleMosaics) {
		// Assert:
		AssertCanMapAccountState(Height(0), { { Xem_Id, Amount(234) }, { MosaicId(1357), Amount(345) }, { MosaicId(31), Amount(45) } });
	}

	TEST(AccountStateMapperTests, CanMapAccountStateWithPublicKeyAndSingleMosaic) {
		// Assert:
		AssertCanMapAccountState(Height(456), { { Xem_Id, Amount(234) } });
	}

	TEST(AccountStateMapperTests, CanMapAccountStateWithPublicKeyAndMultipleMosaics) {
		// Assert:
		AssertCanMapAccountState(Height(456), { { Xem_Id, Amount(234) }, { MosaicId(1357), Amount(345) }, { MosaicId(31), Amount(45) } });
	}

	// endregion

	// region ToAccountState

	namespace {
		void AssertCanMapDbAccountState(Height publicKeyHeight, std::initializer_list<model::Mosaic> mosaics) {
			// Arrange:
			auto state = CreateAccountState(publicKeyHeight, mosaics);
			auto dbAccount = ToDbModel(state);

			// Act:
			std::shared_ptr<state::AccountState> pNewAccountState;
			ToAccountState(dbAccount, [&pNewAccountState](const auto& address, auto height) {
				pNewAccountState = std::make_shared<state::AccountState>(address, height);
				return pNewAccountState;
			});

			// Assert:
			auto accountData = dbAccount.view()["account"].get_document();
			test::AssertEqualAccountState(*pNewAccountState, accountData.view());
		}
	}

	TEST(AccountStateMapperTests, CanMapDbAccountStateWithNeitherPublicKeyNorMosaics) {
		// Assert:
		AssertCanMapDbAccountState(Height(0), {});
	}

	TEST(AccountStateMapperTests, CanMapDbAccountStateWithPublicKeyButWithoutMosaics) {
		// Assert:
		AssertCanMapDbAccountState(Height(456), {});
	}

	TEST(AccountStateMapperTests, CanMapDbAccountStateWithoutPublicKeyButWithSingleMosaic) {
		// Assert:
		AssertCanMapDbAccountState(Height(0), { { Xem_Id, Amount(234) } });
	}

	TEST(AccountStateMapperTests, CanMapDbAccountStateWithoutPublicKeyButWithMultipleMosaics) {
		// Assert:
		AssertCanMapDbAccountState(Height(0), { { Xem_Id, Amount(234) },{ MosaicId(1357), Amount(345) },{ MosaicId(31), Amount(45) } });
	}

	TEST(AccountStateMapperTests, CanMapDbAccountStateWithPublicKeyAndSingleMosaic) {
		// Assert:
		AssertCanMapDbAccountState(Height(456), { { Xem_Id, Amount(234) } });
	}

	TEST(AccountStateMapperTests, CanMapDbAccountStateWithPublicKeyAndMultipleMosaics) {
		// Assert:
		AssertCanMapDbAccountState(Height(456), { { Xem_Id, Amount(234) },{ MosaicId(1357), Amount(345) },{ MosaicId(31), Amount(45) } });
	}

	TEST(AccountStateMapperTests, CanMapDbAccountStateWithImportanceNotSet) {
		// Arrange:
		state::AccountState state(test::GenerateRandomAddress(), Height(123));
		auto dbAccount = ToDbModel(state);

		// Sanity:
		EXPECT_TRUE(dbAccount.view()["account"]["importances"].get_array().value.empty());

		// Act:
		std::shared_ptr<state::AccountState> pNewAccountState;
		ToAccountState(dbAccount, [&pNewAccountState](const auto& address, auto height) {
			pNewAccountState = std::make_shared<state::AccountState>(address, height);
			return pNewAccountState;
		});

		// Assert:
		auto accountData = dbAccount.view()["account"].get_document();
		ASSERT_TRUE(!!pNewAccountState);
		test::AssertEqualAccountState(*pNewAccountState, accountData.view());
	}

	// endregion
}}}
