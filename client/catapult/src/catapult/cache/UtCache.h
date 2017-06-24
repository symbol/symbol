#pragma once
#include "catapult/model/EntityInfo.h"
#include <memory>
#include <vector>

namespace catapult { namespace cache {

	/// An interface for modifying an unconfirmed transactions cache.
	class UtCacheModifier {
	public:
		virtual ~UtCacheModifier() {}

	public:
		/// Adds the transaction info (\a transactionInfo) to the cache.
		/// Returns \c true if the transaction info was successfully added.
		virtual bool add(model::TransactionInfo&& transactionInfo) = 0;

		/// Removes a transaction info identified by \a hash from the cache.
		virtual void remove(const Hash256& hash) = 0;

		/// Removes all transactions from the cache.
		virtual std::vector<model::TransactionInfo> removeAll() = 0;

		/// Prunes transactions with deadlines prior to \a timestamp.
		virtual void prune(Timestamp timestamp) = 0;
	};

	/// A delegating proxy around a UtCacheModifier.
	/// \note This is returned by value by UtCache::modifier in order to allow it to be consistent with other modifier functions.
	class UtCacheModifierProxy final : public utils::MoveOnly {
	public:
		/// Creates a ut cache modifier around \a pModifier.
		explicit UtCacheModifierProxy(std::unique_ptr<UtCacheModifier>&& pModifier) : m_pModifier(std::move(pModifier))
		{}

	public:
		/// Adds the transaction info (\a transactionInfo) to the cache.
		/// Returns \c true if the transaction info was successfully added.
		bool add(model::TransactionInfo&& transactionInfo) {
			return m_pModifier->add(std::move(transactionInfo));
		}

		/// Removes a transaction info identified by \a hash from the cache.
		void remove(const Hash256& hash) {
			return m_pModifier->remove(hash);
		}

		/// Removes all transactions from the cache.
		std::vector<model::TransactionInfo> removeAll() {
			return m_pModifier->removeAll();
		}

		/// Prunes transactions with deadlines prior to \a timestamp.
		void prune(Timestamp timestamp) {
			return m_pModifier->prune(timestamp);
		}

	private:
		std::unique_ptr<UtCacheModifier> m_pModifier;
	};

	/// An interface for caching unconfirmed transactions.
	class UtCache {
	public:
		virtual ~UtCache() {}

	public:
		/// Gets a write only view of the cache.
		virtual UtCacheModifierProxy modifier() = 0;
	};
}}
