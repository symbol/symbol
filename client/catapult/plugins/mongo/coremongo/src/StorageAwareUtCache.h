#pragma once
#include "catapult/cache/UtCache.h"
#include <memory>

namespace catapult { namespace io { class TransactionStorage; } }

namespace catapult { namespace mongo { namespace plugins {

	/// Cache for unconfirmed transactions that is aware of an associated storage.
	class StorageAwareUtCache : public cache::UtCache {
	public:
		/// Creates a db aware unconfirmed transactions cache around an unconfirmed transactions cache
		/// (\a pUnconfirmedTransactionsCache) and a transaction storage (\a pStorage).
		explicit StorageAwareUtCache(
				const std::shared_ptr<cache::UtCache>& pUnconfirmedTransactionsCache,
				const std::shared_ptr<io::TransactionStorage>& pStorage);

	public:
		cache::UtCacheModifierProxy modifier() override;

	private:
		std::shared_ptr<cache::UtCache> m_pUnconfirmedTransactionsCache;
		std::shared_ptr<io::TransactionStorage> m_pStorage;
	};
}}}
