#pragma once
#include "MongoTransactionMetadata.h"
#include "catapult/model/TransactionRegistry.h"
#include "catapult/plugins.h"
#include <mongocxx/client.hpp>

namespace catapult {
	namespace model {
		struct EmbeddedTransaction;
		struct Transaction;
	}
}

namespace catapult { namespace mongo {

	/// A typed mongo transaction plugin.
	template<typename TTransaction>
	class MongoTransactionPluginT {
	public:
		virtual ~MongoTransactionPluginT() {}

	public:
		/// Gets the transaction entity type.
		virtual model::EntityType type() const = 0;

		/// Streams a \a transaction to \a builder.
		virtual void streamTransaction(bsoncxx::builder::stream::document& builder, const TTransaction& transaction) const = 0;
	};

	/// An embedded mongo transaction plugin.
	class EmbeddedMongoTransactionPlugin : public MongoTransactionPluginT<model::EmbeddedTransaction> {
	};

	/// A mongo transaction plugin.
	class MongoTransactionPlugin : public MongoTransactionPluginT<model::Transaction> {
	public:
		/// Extracts dependent documents from \a transaction given the associated \a metadata.
		/// \note The document representing the transaction is created separately via the streamTransaction() call.
		virtual std::vector<bsoncxx::document::value> extractDependentDocuments(
				const model::Transaction& transaction,
				const MongoTransactionMetadata& metadata) const = 0;

		/// \c true if this transaction type supports embedding.
		virtual bool supportsEmbedding() const = 0;

		/// Gets the corresponding embedded plugin if supportsEmbedding() is \c true.
		virtual const EmbeddedMongoTransactionPlugin& embeddedPlugin() const = 0;
	};

	/// A registry of mongo transaction plugins.
	class MongoTransactionRegistry : public model::TransactionRegistryT<MongoTransactionPlugin> {};
}}
