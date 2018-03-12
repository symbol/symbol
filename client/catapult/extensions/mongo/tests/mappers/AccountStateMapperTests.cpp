#include "mongo/src/mappers/AccountStateMapper.h"
#include "catapult/model/Mosaic.h"
#include "catapult/state/AccountState.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/TestHarness.h"
#include <mongocxx/client.hpp>

namespace catapult { namespace mongo { namespace mappers {

#define TEST_CLASS AccountStateMapperTests

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

	TEST(TEST_CLASS, CanMapAccountStateWithNeitherPublicKeyNorMosaics) {
		// Assert:
		AssertCanMapAccountState(Height(0), {});
	}

	TEST(TEST_CLASS, CanMapAccountStateWithPublicKeyButWithoutMosaics) {
		// Assert:
		AssertCanMapAccountState(Height(456), {});
	}

	TEST(TEST_CLASS, CanMapAccountStateWithoutPublicKeyButWithSingleMosaic) {
		// Assert:
		AssertCanMapAccountState(Height(0), { { Xem_Id, Amount(234) } });
	}

	TEST(TEST_CLASS, CanMapAccountStateWithoutPublicKeyButWithMultipleMosaics) {
		// Assert:
		AssertCanMapAccountState(Height(0), { { Xem_Id, Amount(234) }, { MosaicId(1357), Amount(345) }, { MosaicId(31), Amount(45) } });
	}

	TEST(TEST_CLASS, CanMapAccountStateWithPublicKeyAndSingleMosaic) {
		// Assert:
		AssertCanMapAccountState(Height(456), { { Xem_Id, Amount(234) } });
	}

	TEST(TEST_CLASS, CanMapAccountStateWithPublicKeyAndMultipleMosaics) {
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
			state::AccountState newAccountState(Address(), Height(0));
			ToAccountState(dbAccount, [&newAccountState](const auto& address, auto height) -> state::AccountState& {
				newAccountState.Address = address;
				newAccountState.AddressHeight = height;
				return newAccountState;
			});

			// Assert:
			auto accountData = dbAccount.view()["account"].get_document();
			test::AssertEqualAccountState(newAccountState, accountData.view());
		}
	}

	TEST(TEST_CLASS, CanMapDbAccountStateWithNeitherPublicKeyNorMosaics) {
		// Assert:
		AssertCanMapDbAccountState(Height(0), {});
	}

	TEST(TEST_CLASS, CanMapDbAccountStateWithPublicKeyButWithoutMosaics) {
		// Assert:
		AssertCanMapDbAccountState(Height(456), {});
	}

	TEST(TEST_CLASS, CanMapDbAccountStateWithoutPublicKeyButWithSingleMosaic) {
		// Assert:
		AssertCanMapDbAccountState(Height(0), { { Xem_Id, Amount(234) } });
	}

	TEST(TEST_CLASS, CanMapDbAccountStateWithoutPublicKeyButWithMultipleMosaics) {
		// Assert:
		AssertCanMapDbAccountState(Height(0), { { Xem_Id, Amount(234) }, { MosaicId(1357), Amount(345) }, { MosaicId(31), Amount(45) } });
	}

	TEST(TEST_CLASS, CanMapDbAccountStateWithPublicKeyAndSingleMosaic) {
		// Assert:
		AssertCanMapDbAccountState(Height(456), { { Xem_Id, Amount(234) } });
	}

	TEST(TEST_CLASS, CanMapDbAccountStateWithPublicKeyAndMultipleMosaics) {
		// Assert:
		AssertCanMapDbAccountState(
				Height(456),
				{ { Xem_Id, Amount(234) }, { MosaicId(1357), Amount(345) }, { MosaicId(31), Amount(45) } });
	}

	TEST(TEST_CLASS, CanMapDbAccountStateWithImportanceNotSet) {
		// Arrange:
		state::AccountState state(test::GenerateRandomAddress(), Height(123));
		auto dbAccount = ToDbModel(state);

		// Sanity:
		EXPECT_TRUE(dbAccount.view()["account"]["importances"].get_array().value.empty());

		// Act:
		state::AccountState newAccountState(Address(), Height(0));
		ToAccountState(dbAccount, [&newAccountState](const auto& address, auto height) -> state::AccountState& {
			newAccountState.Address = address;
			newAccountState.AddressHeight = height;
			return newAccountState;
		});

		// Assert:
		auto accountData = dbAccount.view()["account"].get_document();
		test::AssertEqualAccountState(newAccountState, accountData.view());
	}

	// endregion
}}}
