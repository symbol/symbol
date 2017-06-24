#pragma once
#include "TransactionRegistry.h"
#include "catapult/model/WeakEntityInfo.h"
#include "catapult/types.h"

namespace catapult {
	namespace model {
		struct EmbeddedEntity;
		class NotificationSubscriber;
		struct Transaction;
	}
}

namespace catapult { namespace model {

	/// A typed transaction plugin.
	template<typename TTransaction>
	class TransactionPluginT {
	public:
		virtual ~TransactionPluginT() {}

	public:
		/// Gets the transaction entity type.
		virtual EntityType type() const = 0;

		/// Calculates the real size of \a transaction.
		virtual uint64_t calculateRealSize(const TTransaction& transaction) const = 0;
	};

	/// An embedded transaction plugin.
	class EmbeddedTransactionPlugin : public TransactionPluginT<EmbeddedEntity> {
	public:
		/// Sends all notifications from \a transaction to \a sub.
		virtual void publish(const EmbeddedEntity& transaction, NotificationSubscriber& sub) const = 0;
	};

	/// A transaction plugin.
	class TransactionPlugin : public TransactionPluginT<Transaction> {
	public:
		/// Sends all notifications from \a transactionInfo to \a sub.
		virtual void publish(const WeakEntityInfoT<Transaction>& transactionInfo, NotificationSubscriber& sub) const = 0;

		/// Extracts the primary data buffer from \a transaction that is used for signing and basic hashing.
		virtual RawBuffer dataBuffer(const Transaction& transaction) const = 0;

		/// Extracts additional buffers from \a transaction that should be included in the merkle hash in addition to
		/// the primary data buffer.
		virtual std::vector<RawBuffer> merkleSupplementaryBuffers(const Transaction& transaction) const = 0;

		/// \c true if this transaction type supports embedding.
		virtual bool supportsEmbedding() const = 0;

		/// Gets the corresponding embedded plugin if supportsEmbedding() is \c true.
		virtual const EmbeddedTransactionPlugin& embeddedPlugin() const = 0;
	};

	/// A registry of transaction plugins.
	class TransactionRegistry : public TransactionRegistryT<TransactionPlugin> {
	};
}}
