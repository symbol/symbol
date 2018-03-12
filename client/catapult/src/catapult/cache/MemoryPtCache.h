#pragma once
#include "MemoryCacheOptions.h"
#include "MemoryCacheProxy.h"
#include "PtCache.h"
#include "ShortHashPair.h"
#include "catapult/model/CosignedTransactionInfo.h"
#include "catapult/model/WeakCosignedTransactionInfo.h"
#include "catapult/utils/Hashers.h"
#include "catapult/utils/SpinReaderWriterLock.h"
#include <unordered_map>

namespace catapult { namespace cache { class PtData; } }

namespace catapult { namespace cache {

	using PtDataContainer = std::unordered_map<Hash256, PtData, utils::ArrayHasher<Hash256>>;

	/// A read only view on top of partial transactions cache.
	class MemoryPtCacheView {
	private:
		using UnknownTransactionInfos = std::vector<model::CosignedTransactionInfo>;

	public:
		/// Creates a view around around a maximum response size (\a maxResponseSize), a partial transaction data container
		/// (\a transactionDataContainer) with lock context \a readLock.
		explicit MemoryPtCacheView(
				uint64_t maxResponseSize,
				const PtDataContainer& transactionDataContainer,
				utils::SpinReaderWriterLock::ReaderLockGuard&& readLock);

	public:
		/// Returns the number of partial transactions in the cache.
		size_t size() const;

		/// Finds a partial transaction in the cache with associated \a hash or returns \c nullptr if no such transaction exists.
		model::WeakCosignedTransactionInfo find(const Hash256& hash) const;

		/// Gets a range of short hash pairs of all transactions in the cache.
		/// A short hash pair consists of the first 4 bytes of the transaction hash and the first 4 bytes of the cosignature hash.
		ShortHashPairRange shortHashPairs() const;

		/// Gets a vector of all unknown transaction infos in the cache that do not have a short hash pair in \a knownShortHashPairs.
		UnknownTransactionInfos unknownTransactions(const ShortHashPairMap& knownShortHashPairs) const;

	private:
		uint64_t m_maxResponseSize;
		const PtDataContainer& m_transactionDataContainer;
		utils::SpinReaderWriterLock::ReaderLockGuard m_readLock;
	};

	/// Cache for all partial transactions.
	class MemoryPtCache : public PtCache {
	public:
		/// Creates a partial transactions cache around \a options.
		explicit MemoryPtCache(const MemoryCacheOptions& options);

		/// Destroys a partial transactions cache.
		~MemoryPtCache() override;

	public:
		/// Gets a read only view based on this cache.
		MemoryPtCacheView view() const;

	public:
		PtCacheModifierProxy modifier() override;

	private:
		struct Impl;

	private:
		MemoryCacheOptions m_options;
		std::unique_ptr<Impl> m_pImpl;
		mutable utils::SpinReaderWriterLock m_lock;
	};

	/// A delegating proxy around a MemoryPtCache.
	class MemoryPtCacheProxy : public MemoryCacheProxy<MemoryPtCache, PtCache, PtCacheModifierProxy> {
		using MemoryCacheProxy<MemoryPtCache, PtCache, PtCacheModifierProxy>::MemoryCacheProxy;
	};
}}
