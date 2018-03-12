#pragma once
#include "TransactionsInfo.h"
#include "UnlockedAccounts.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/model/Elements.h"
#include "catapult/model/EntityInfo.h"

namespace catapult { namespace harvesting {

	/// A class that creates new blocks.
	class Harvester {
	public:
		/// Creates a harvester around a catapult \a cache, a block chain \a config, an unlocked accounts set (\a unlockedAccounts)
		/// and a transactions info supplier (\a transactionsInfoSupplier).
		explicit Harvester(
				const cache::CatapultCache& cache,
				const model::BlockChainConfiguration& config,
				const UnlockedAccounts& unlockedAccounts,
				const TransactionsInfoSupplier& transactionsInfoSupplier);

	public:
		/// Creates the best block (if any) harvested by any unlocked account.
		/// The new block has \a lastBlockElement as parent and \a timestamp as timestamp.
		std::unique_ptr<model::Block> harvest(const model::BlockElement& lastBlockElement, Timestamp timestamp);

	private:
		const cache::CatapultCache& m_cache;
		const model::BlockChainConfiguration m_config;
		const UnlockedAccounts& m_unlockedAccounts;
		TransactionsInfoSupplier m_transactionsInfoSupplier;
	};
}}
