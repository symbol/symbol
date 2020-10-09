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

#include "mongo/src/storages/MongoAccountStateCacheStorage.h"
#include "mongo/src/mappers/MapperUtils.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/Address.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "mongo/tests/test/MapperTestUtils.h"
#include "mongo/tests/test/MongoFlatCacheStorageTests.h"
#include "tests/test/cache/CacheTestUtils.h"
#include "tests/test/core/AccountStateTestUtils.h"
#include "tests/TestHarness.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace storages {

#define TEST_CLASS MongoAccountStateCacheStorageTests

	namespace {
		constexpr auto Currency_Mosaic_Id = MosaicId(1234);

		struct AccountStateCacheTraits {
			using CacheType = cache::AccountStateCache;
			using ModelType = state::AccountState;

			static constexpr auto Collection_Name = "accounts";
			static constexpr auto Network_Id = static_cast<model::NetworkIdentifier>(0x5A);
			static constexpr auto CreateCacheStorage = CreateMongoAccountStateCacheStorage;

			static cache::CatapultCache CreateCache() {
				auto chainConfig = model::BlockChainConfiguration::Uninitialized();
				chainConfig.Network.Identifier = model::NetworkIdentifier::Private_Test;
				return test::CreateEmptyCatapultCache(chainConfig);
			}

			static ModelType GenerateRandomElement(uint32_t id) {
				auto height = Height(id);
				auto publicKey = test::GenerateRandomByteArray<Key>();
				auto address = model::PublicKeyToAddress(publicKey, model::NetworkIdentifier::Private_Test);
				auto accountState = state::AccountState(address, Height(1234567) + height);
				accountState.PublicKey = publicKey;
				accountState.PublicKeyHeight = Height(1234567) + height;
				auto randomAmount = Amount((test::Random() % 1'000'000 + 1'000) * 1'000'000);
				auto randomImportance = Importance(test::Random() % 1'000'000'000 + 1'000'000'000);
				auto randomImportanceHeight = test::GenerateRandomValue<model::ImportanceHeight>();
				accountState.Balances.credit(Currency_Mosaic_Id, randomAmount);
				accountState.ImportanceSnapshots.set(randomImportance, randomImportanceHeight);
				return accountState;
			}

			static void Add(cache::CatapultCacheDelta& delta, const ModelType& accountState) {
				auto& accountStateCacheDelta = delta.sub<cache::AccountStateCache>();
				accountStateCacheDelta.addAccount(accountState.PublicKey, accountState.PublicKeyHeight);
				auto& accountStateFromCache = accountStateCacheDelta.find(accountState.PublicKey).get();
				accountStateFromCache.Balances.credit(Currency_Mosaic_Id, accountState.Balances.get(Currency_Mosaic_Id));

				auto height = accountState.ImportanceSnapshots.height();
				accountStateFromCache.ImportanceSnapshots.set(accountState.ImportanceSnapshots.get(height), height);
			}

			static void Remove(cache::CatapultCacheDelta& delta, const ModelType& accountState) {
				auto& accountStateCacheDelta = delta.sub<cache::AccountStateCache>();
				accountStateCacheDelta.queueRemove(accountState.PublicKey, accountState.PublicKeyHeight);
				accountStateCacheDelta.commitRemovals();
			}

			static void Mutate(cache::CatapultCacheDelta& delta, ModelType& accountState) {
				// update expected
				accountState.Balances.credit(Currency_Mosaic_Id, Amount(12'345'000'000));

				// update cache
				auto& accountStateCacheDelta = delta.sub<cache::AccountStateCache>();
				auto& accountStateFromCache = accountStateCacheDelta.find(accountState.PublicKey).get();
				accountStateFromCache.Balances.credit(Currency_Mosaic_Id, Amount(12'345'000'000));
			}

			static auto GetFindFilter(const ModelType& accountState) {
				return document() << "account.address" << mappers::ToBinary(accountState.Address) << finalize;
			}

			static void AssertEqual(const ModelType& accountState, const bsoncxx::document::view& view) {
				test::AssertEqualAccountState(accountState, view["account"].get_document().view());
			}
		};
	}

	DEFINE_FLAT_CACHE_STORAGE_TESTS(AccountStateCacheTraits,)
}}}
