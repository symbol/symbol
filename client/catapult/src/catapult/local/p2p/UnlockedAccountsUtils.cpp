#include "UnlockedAccountsUtils.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache/ImportanceView.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/chain/UnlockedAccounts.h"

namespace catapult { namespace local { namespace p2p {

	void PruneUnlockedAccounts(
			chain::UnlockedAccounts& unlockedAccounts,
			const cache::CatapultCache& cache,
			Amount minHarvesterBalance) {
		auto cacheView = cache.createView();
		auto height = cacheView.height() + Height(1);
		auto readOnlyAccountStateCache = cache::ReadOnlyAccountStateCache(cacheView.sub<cache::AccountStateCache>());
		unlockedAccounts.modifier().removeIf([height, minHarvesterBalance, &readOnlyAccountStateCache](const auto& key) {
			cache::ImportanceView view(readOnlyAccountStateCache);
			return !view.canHarvest(key, height, minHarvesterBalance);
		});
	}
}}}
