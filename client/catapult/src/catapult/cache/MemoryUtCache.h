#pragma once
#include "UtCache.h"
#include "catapult/model/RangeTypes.h"
#include "catapult/utils/Hashers.h"
#include "catapult/utils/SpinReaderWriterLock.h"
#include <set>
#include <unordered_map>

namespace catapult { namespace cache { struct TransactionData; } }

namespace catapult { namespace cache {
	/// Options for customizing the behavior of the unconfirmed transactions cache.
	class MemoryUtCacheOptions {
	public:
		/// Creates default options.
		constexpr MemoryUtCacheOptions() : MemoryUtCacheOptions(0, 0)
		{}

		/// Creates options with custom \a maxResponseSize and \a maxCacheSize.
		constexpr MemoryUtCacheOptions(uint64_t maxResponseSize, uint64_t maxCacheSize)
				: MaxResponseSize(maxResponseSize)
				, MaxCacheSize(maxCacheSize)
		{}

	public:
		/// The maximum response size.
		uint64_t MaxResponseSize;

		/// The maximum size of the cache.
		uint64_t MaxCacheSize;
	};

	/// A read only view on top of unconfirmed transactions cache.
	class MemoryUtCacheView {
	private:
		using UnknownTransactions = std::vector<std::shared_ptr<const model::Transaction>>;
		using IdLookup = std::unordered_map<Hash256, size_t, utils::ArrayHasher<Hash256>>;
		using TransactionInfoConsumer = std::function<bool (const model::TransactionInfo&)>;

	public:
		/// Creates a view around a maximum response size (\a maxResonseSize), a transaction data set (\a transactionDataSet)
		/// and an id lookup (\a idLookup) with lock context \a readLock.
		explicit MemoryUtCacheView(
				uint64_t maxResponseSize,
				const std::set<TransactionData>& transactionDataSet,
				const IdLookup& idLookup,
				utils::SpinReaderWriterLock::ReaderLockGuard&& readLock);

	public:
		/// Returns the number of unconfirmed transactions in the cache.
		size_t size() const;

		/// Returns \c true if the cache contains a transaction info with associated \a hash, \c false otherwise.
		bool contains(const Hash256& hash) const;

		/// Calls \a consumer with all transaction infos until all are consumed or \c false is returned by consumer.
		void forEach(const TransactionInfoConsumer& consumer) const;

		/// Gets a range of short hashes of all transactions in the cache.
		/// A short hash consists of the first 4 bytes of the complete hash.
		model::ShortHashRange shortHashes() const;

		/// Gets a vector of all transactions from the cache whose short hash is not
		/// in \a knownShortHashes.
		UnknownTransactions unknownTransactions(const utils::ShortHashesSet& knownShortHashes) const;

	private:
		uint64_t m_maxResponseSize;
		const std::set<TransactionData>& m_transactionDataSet;
		const IdLookup& m_idLookup;
		utils::SpinReaderWriterLock::ReaderLockGuard m_readLock;
	};

	/// Cache for all unconfirmed transactions.
	class MemoryUtCache : public UtCache {
	public:
		/// Creates an unconfirmed transactions cache around \a options.
		explicit MemoryUtCache(const MemoryUtCacheOptions& options);

		/// Destroys an unconfirmed transactions cache.
		~MemoryUtCache() override;

	public:
		/// Gets a read only view based on this cache.
		MemoryUtCacheView view() const;

	public:
		UtCacheModifierProxy modifier() override;

	private:
		struct Impl;

	private:
		MemoryUtCacheOptions m_options;
		size_t m_idSequence;
		std::unique_ptr<Impl> m_pImpl;
		mutable utils::SpinReaderWriterLock m_lock;
	};
}}
