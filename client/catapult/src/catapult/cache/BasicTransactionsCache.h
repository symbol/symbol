#pragma once
#include "catapult/model/ContainerTypes.h"
#include "catapult/model/EntityInfo.h"
#include <memory>

namespace catapult { namespace cache {

	/// An interface for modifying a transactions cache.
	template<typename TTransactionInfo>
	class BasicTransactionsCacheModifier {
	public:
		virtual ~BasicTransactionsCacheModifier() noexcept(false) {}

	public:
		/// Adds the transaction info (\a transactionInfo) to the cache.
		/// Returns \c true if the transaction info was successfully added.
		virtual bool add(const TTransactionInfo& transactionInfo) = 0;

		/// Removes the transaction identified by \a hash from the cache.
		virtual TTransactionInfo remove(const Hash256& hash) = 0;
	};

	/// A delegating proxy around a transactions cache modifier.
	/// \note This is returned by value by BasicTransactionsCache::modifier in order to allow it to be consistent with other
	///       modifier functions.
	template<typename TTransactionInfo, typename TTransactionsCacheModifier>
	class BasicTransactionsCacheModifierProxy : public utils::MoveOnly {
	public:
		/// Creates a transactions cache modifier around \a pModifier.
		explicit BasicTransactionsCacheModifierProxy(std::unique_ptr<TTransactionsCacheModifier>&& pModifier)
				: m_pModifier(std::move(pModifier))
		{}

	public:
		/// Adds the transaction info (\a transactionInfo) to the cache.
		/// Returns \c true if the transaction info was successfully added.
		bool add(const TTransactionInfo& transactionInfo) {
			return m_pModifier->add(transactionInfo);
		}

		/// Removes the transaction identified by \a hash from the cache.
		TTransactionInfo remove(const Hash256& hash) {
			return m_pModifier->remove(hash);
		}

	protected:
		/// Gets the modifier.
		TTransactionsCacheModifier& modifier() {
			return *m_pModifier;
		}

	private:
		std::unique_ptr<TTransactionsCacheModifier> m_pModifier;
	};

	/// An interface for caching transactions.
	template<typename TTransactionsCacheModifierProxy>
	class BasicTransactionsCache : public utils::NonCopyable {
	public:
		virtual ~BasicTransactionsCache() {}

	public:
		/// Gets a write only view of the cache.
		virtual TTransactionsCacheModifierProxy modifier() = 0;
	};
}}
