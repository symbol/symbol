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
		struct AccountStateCacheTraits {
			using CacheType = cache::AccountStateCache;
			using ModelType = std::shared_ptr<state::AccountState>;

			static constexpr auto Collection_Name = "accounts";
			static constexpr auto Network_Id = static_cast<model::NetworkIdentifier>(0x5A);
			static constexpr auto CreateCacheStorage = CreateMongoAccountStateCacheStorage;

			static cache::CatapultCache CreateCache() {
				auto chainConfig = model::BlockChainConfiguration::Uninitialized();
				chainConfig.Network.Identifier = model::NetworkIdentifier::Mijin_Test;
				return test::CreateEmptyCatapultCache(chainConfig);
			}

			static ModelType GenerateRandomElement(uint32_t id) {
				auto height = Height(id);
				auto publicKey = test::GenerateRandomData<Key_Size>();
				auto address = model::PublicKeyToAddress(publicKey, model::NetworkIdentifier::Mijin_Test);
				auto pState = std::make_shared<state::AccountState>(address, Height(1234567) + height);
				pState->PublicKey = publicKey;
				pState->PublicKeyHeight = Height(1234567) + height;
				auto randomAmount = Amount((test::Random() % 1'000'000 + 1'000) * 1'000'000);
				auto randomImportance = Importance(test::Random() % 1'000'000'000 + 1'000'000'000);
				auto randomImportanceHeight = test::GenerateRandomValue<model::ImportanceHeight>();
				pState->Balances.credit(Xem_Id, randomAmount);
				pState->ImportanceInfo.set(randomImportance, randomImportanceHeight);
				return pState;
			}

			static void Add(cache::CatapultCacheDelta& delta, const ModelType& pAccountState) {
				auto& accountStateCacheDelta = delta.sub<cache::AccountStateCache>();
				auto& accountState = accountStateCacheDelta.addAccount(pAccountState->PublicKey, pAccountState->PublicKeyHeight);
				accountState.Balances.credit(Xem_Id, pAccountState->Balances.get(Xem_Id));

				auto height = pAccountState->ImportanceInfo.height();
				accountState.ImportanceInfo.set(pAccountState->ImportanceInfo.get(height), height);
			}

			static void Remove(cache::CatapultCacheDelta& delta, const ModelType& pAccountState) {
				auto& accountStateCacheDelta = delta.sub<cache::AccountStateCache>();
				accountStateCacheDelta.queueRemove(pAccountState->PublicKey, pAccountState->PublicKeyHeight);
				accountStateCacheDelta.commitRemovals();
			}

			static void Mutate(cache::CatapultCacheDelta& delta, const ModelType& pAccountState) {
				// update expected
				pAccountState->Balances.credit(Xem_Id, Amount(12'345'000'000u));

				// update cache
				auto& accountStateCacheDelta = delta.sub<cache::AccountStateCache>();
				auto& accountStateFromCache = accountStateCacheDelta.get(pAccountState->PublicKey);
				accountStateFromCache.Balances.credit(Xem_Id, Amount(12'345'000'000u));
			}

			static auto GetFindFilter(const ModelType& pAccountState) {
				return document() << "account.address" << mappers::ToBinary(pAccountState->Address) << finalize;
			}

			static void AssertEqual(const ModelType& pAccountState, const bsoncxx::document::view& view) {
				test::AssertEqualAccountState(*pAccountState, view["account"].get_document().view());
			}
		};
	}

	DEFINE_FLAT_CACHE_STORAGE_TESTS(AccountStateCacheTraits,);
}}}
