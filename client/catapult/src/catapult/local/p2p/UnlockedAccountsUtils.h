#pragma once
#include "catapult/types.h"

namespace catapult {
	namespace cache { class CatapultCache; }
	namespace chain { class UnlockedAccounts; }
}

namespace catapult { namespace local { namespace p2p {

	/// Prunes \a unlockedAccounts given \a cache and \a minHarvesterBalance.
	void PruneUnlockedAccounts(
			chain::UnlockedAccounts& unlockedAccounts,
			const cache::CatapultCache& cache,
			Amount minHarvesterBalance);
}}}
