#pragma once
#include "MongoStorageConfiguration.h"
#include "catapult/io/TransactionStorage.h"
#include "catapult/utils/Hashers.h"
#include <unordered_set>

namespace catapult { namespace mongo { namespace plugins { class MongoTransactionRegistry; } } }

namespace catapult { namespace mongo { namespace plugins {

	/// Mongodb backed transaction storage.
	class MongoDbTransactionStorage final : public io::TransactionStorage {
	private:
		struct TransactionInfoHasher {
			size_t operator()(const model::TransactionInfo& info) const {
				return utils::ArrayHasher<Hash256>()(info.EntityHash);
			}
		};

		struct TransactionInfoComparer {
			bool operator()(const model::TransactionInfo& lhs, const model::TransactionInfo& rhs) const {
				return lhs.EntityHash == rhs.EntityHash;
			}
		};

	public:
		/// Creates a mongodb transaction storage around \a pConfig and \a pTransactionRegistry.
		explicit MongoDbTransactionStorage(
				const std::shared_ptr<MongoStorageConfiguration>& pConfig,
				const std::shared_ptr<const MongoTransactionRegistry>& pTransactionRegistry);

	public:
		bool saveTransaction(const model::TransactionInfo& transactionInfo) override;

		void removeTransaction(const Hash256& hash) override;

		void removeTransactions(const std::vector<model::TransactionInfo>& transactionInfos) override;

		void pruneTransactions(Timestamp timestamp) override;

		void commit() override;

	private:
		void commitInserts();
		void commitDeletes();

	private:
		std::shared_ptr<MongoStorageConfiguration> m_pConfig;
		std::shared_ptr<const MongoTransactionRegistry> m_pTransactionRegistry;
		MongoDatabase m_database;
		std::unordered_set<model::TransactionInfo, TransactionInfoHasher, TransactionInfoComparer> m_addedTransactions;
		std::unordered_set<Hash256, utils::ArrayHasher<Hash256>> m_deletedTransactionHashes;
	};
}}}
