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
#include "tests/test/core/AccountStateTestUtils.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/TestHarness.h"
#include <mongocxx/client.hpp>

namespace catapult { namespace mongo { namespace mappers {

#define TEST_CLASS AccountStateMapperTests

	// region ToDbModel

	namespace {
		struct RandomSeed {
			state::AccountPublicKeys::KeyType SupplementalPublicKeysMask = state::AccountPublicKeys::KeyType::Unset;
			uint8_t NumVotingKeys = 0;
		};

		auto CreateAccountState(Height publicKeyHeight, std::initializer_list<model::Mosaic> mosaics, const RandomSeed& seed) {
			state::AccountState accountState(test::GenerateRandomAddress(), Height(123));
			if (Height(0) != publicKeyHeight) {
				accountState.PublicKeyHeight = publicKeyHeight;
				test::FillWithRandomData(accountState.PublicKey);
			}

			accountState.AccountType = static_cast<state::AccountType>(34);
			test::SetRandomSupplementalPublicKeys(accountState, seed.SupplementalPublicKeysMask, seed.NumVotingKeys);

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

		void AssertCanMapAccountState(
				Height publicKeyHeight,
				std::initializer_list<model::Mosaic> mosaics,
				const RandomSeed& seed = RandomSeed()) {
			// Arrange:
			auto accountState = CreateAccountState(publicKeyHeight, mosaics, seed);

			// Act:
			auto dbAccount = ToDbModel(accountState);

			// Assert:
			auto view = dbAccount.view();
			EXPECT_EQ(1u, test::GetFieldCount(view));

			auto accountView = view["account"].get_document().view();
			EXPECT_EQ(9u, test::GetFieldCount(accountView));
			test::AssertEqualAccountState(accountState, accountView);
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

	namespace {
		void ForEachRandomSeed(const consumer<const RandomSeed&>& action) {
			auto accountPublicKeysMasks = std::initializer_list<state::AccountPublicKeys::KeyType>{
				state::AccountPublicKeys::KeyType::Unset,
				state::AccountPublicKeys::KeyType::Linked,
				state::AccountPublicKeys::KeyType::Node,
				state::AccountPublicKeys::KeyType::VRF,
				state::AccountPublicKeys::KeyType::Linked | state::AccountPublicKeys::KeyType::Node,
				state::AccountPublicKeys::KeyType::All
			};

			for (auto keyType : accountPublicKeysMasks) {
				for (auto numVotingKeys : std::initializer_list<uint8_t>{ 0, 3 }) {
					CATAPULT_LOG(debug)
							<< "key type mask: " << static_cast<uint16_t>(keyType)
							<< ", num voting keys: " << static_cast<uint16_t>(numVotingKeys);

					action({ keyType, numVotingKeys });
				}
			}
		}
	}

	TEST(TEST_CLASS, CanMapAccountStateWithSupplementalPublicKeys) {
		// Arrange:
		ForEachRandomSeed([](const auto& seed) {
			// Act + Assert:
			AssertCanMapAccountState(Height(456), { { MosaicId(1234), Amount(234) } }, seed);
		});
	}

	// endregion
}}}
