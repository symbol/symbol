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
		void AssertEqualAccountStateMetadata(const bsoncxx::document::view& dbMetadata) {
			EXPECT_EQ(0u, test::GetFieldCount(dbMetadata));
		}

		auto CreateAccountState(Height publicKeyHeight, std::initializer_list<model::Mosaic> mosaics) {
			state::AccountState accountState(test::GenerateRandomAddress(), Height(123));
			if (Height(0) != publicKeyHeight) {
				accountState.PublicKeyHeight = publicKeyHeight;
				test::FillWithRandomData(accountState.PublicKey);
			}

			accountState.AccountType = static_cast<state::AccountType>(34);
			test::FillWithRandomData(accountState.LinkedAccountKey);

			auto numImportanceSnapshots = 1u + test::Random() % Importance_History_Size;
			for (auto i = 0u; i < numImportanceSnapshots; ++i)
				accountState.ImportanceSnapshots.set(Importance(234 + i * 100), model::ImportanceHeight(345 + i * 10));

			auto numActivityBuckets = 1u + test::Random() % Activity_Bucket_History_Size;
			for (auto i = 0u; i < numActivityBuckets; ++i) {
				accountState.ActivityBuckets.update(model::ImportanceHeight(345 + i * 10), [i](auto& bucket) {
					bucket.TotalFeesPaid = Amount(2 * i);
					bucket.BeneficiaryCount = i * i;
					bucket.RawScore = i * i * i;
				});
			}

			for (const auto& mosaic : mosaics)
				accountState.Balances.credit(mosaic.MosaicId, mosaic.Amount);

			return accountState;
		}

		void AssertCanMapAccountState(Height publicKeyHeight, std::initializer_list<model::Mosaic> mosaics) {
			// Arrange:
			auto accountState = CreateAccountState(publicKeyHeight, mosaics);

			// Act:
			auto dbAccount = ToDbModel(accountState);

			// Assert:
			auto view = dbAccount.view();
			EXPECT_EQ(2u, test::GetFieldCount(view));

			auto metaView = view["meta"].get_document().view();
			AssertEqualAccountStateMetadata(metaView);

			auto account = view["account"].get_document().view();
			EXPECT_EQ(9u, test::GetFieldCount(account));
			test::AssertEqualAccountState(accountState, account);
		}
	}

	TEST(TEST_CLASS, CanMapAccountStateWithNeitherPublicKeyNorMosaics) {
		AssertCanMapAccountState(Height(0), {});
	}

	TEST(TEST_CLASS, CanMapAccountStateWithPublicKeyButWithoutMosaics) {
		AssertCanMapAccountState(Height(456), {});
	}

	TEST(TEST_CLASS, CanMapAccountStateWithoutPublicKeyButWithSingleMosaic) {
		AssertCanMapAccountState(Height(0), { { MosaicId(1234), Amount(234) } });
	}

	TEST(TEST_CLASS, CanMapAccountStateWithoutPublicKeyButWithMultipleMosaics) {
		AssertCanMapAccountState(
				Height(0),
				{ { MosaicId(1234), Amount(234) }, { MosaicId(1357), Amount(345) }, { MosaicId(31), Amount(45) } });
	}

	TEST(TEST_CLASS, CanMapAccountStateWithPublicKeyAndSingleMosaic) {
		AssertCanMapAccountState(Height(456), { { MosaicId(1234), Amount(234) } });
	}

	TEST(TEST_CLASS, CanMapAccountStateWithPublicKeyAndMultipleMosaics) {
		AssertCanMapAccountState(
				Height(456),
				{ { MosaicId(1234), Amount(234) }, { MosaicId(1357), Amount(345) }, { MosaicId(31), Amount(45) } });
	}

	// endregion
}}}
