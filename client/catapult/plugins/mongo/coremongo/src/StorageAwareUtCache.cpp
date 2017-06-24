#include "StorageAwareUtCache.h"
#include "catapult/io/TransactionStorage.h"

namespace catapult { namespace mongo { namespace plugins {

	// region StorageAwareUtCacheModifier

	namespace {
		class StorageAwareUtCacheModifier : public cache::UtCacheModifier {
		public:
			explicit StorageAwareUtCacheModifier(
					cache::UtCacheModifierProxy&& modifier,
					io::TransactionStorage& storage)
					: m_modifier(std::move(modifier))
					, m_storage(storage)
			{}

			~StorageAwareUtCacheModifier() {
				m_storage.commit();
			}

		public:
			bool add(model::TransactionInfo&& transactionInfo) override {
				auto copyInfo = transactionInfo.copy();
				return m_modifier.add(std::move(transactionInfo)) && m_storage.saveTransaction(copyInfo);
			}

			void remove(const Hash256& hash) override {
				m_storage.removeTransaction(hash);
				m_modifier.remove(hash);
			}

			std::vector<model::TransactionInfo> removeAll() override {
				auto transactionInfos = m_modifier.removeAll();
				m_storage.removeTransactions(transactionInfos);
				return transactionInfos;
			}

			void prune(Timestamp timestamp) override {
				m_storage.pruneTransactions(timestamp);
				m_modifier.prune(timestamp);
			}

		private:
			cache::UtCacheModifierProxy m_modifier;
			io::TransactionStorage& m_storage;
		};
	}

	// endregion

	// region StorageAwareUtCache

	StorageAwareUtCache::StorageAwareUtCache(
			const std::shared_ptr<cache::UtCache>& pUnconfirmedTransactionsCache,
			const std::shared_ptr<io::TransactionStorage>& pStorage)
			: m_pUnconfirmedTransactionsCache(pUnconfirmedTransactionsCache)
			, m_pStorage(pStorage)
	{}

	cache::UtCacheModifierProxy StorageAwareUtCache::modifier() {
		return cache::UtCacheModifierProxy(std::make_unique<StorageAwareUtCacheModifier>(
				m_pUnconfirmedTransactionsCache->modifier(),
				*m_pStorage));
	}

	// endregion
}}}
