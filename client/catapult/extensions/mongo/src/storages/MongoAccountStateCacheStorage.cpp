#include "MongoAccountStateCacheStorage.h"
#include "mongo/src/mappers/AccountStateMapper.h"
#include "mongo/src/storages/MongoCacheStorage.h"
#include "catapult/cache_core/AccountStateCache.h"

using namespace bsoncxx::builder::stream;

namespace catapult { namespace mongo { namespace storages {

	namespace {
		struct AccountStateCacheTraits {
			using CacheType = cache::AccountStateCache;
			using CacheDeltaType = cache::AccountStateCacheDelta;
			using KeyType = Address;
			using ModelType = state::AccountState;

			static constexpr const char* Collection_Name = "accounts";
			static constexpr const char* Id_Property_Name = "account.address";

			static auto GetId(const ModelType& accountState) {
				return accountState.Address;
			}

			static auto MapToMongoId(const Address& address) {
				return mappers::ToBinary(address);
			}

			static auto MapToMongoDocument(const state::AccountState& accountState, model::NetworkIdentifier) {
				return mappers::ToDbModel(accountState);
			}

			static void Insert(CacheDeltaType& cache, const bsoncxx::document::view& document) {
				mappers::ToAccountState(document, [&cache](const auto& address, auto height) -> state::AccountState& {
					return cache.addAccount(address, height);
				});
			}
		};
	}

	DEFINE_MONGO_FLAT_CACHE_STORAGE(AccountState, AccountStateCacheTraits)
}}}
