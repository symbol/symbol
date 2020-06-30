/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#pragma once
#include "MemoryCacheOptions.h"
#include "MemoryCacheProxy.h"
#include "UtCache.h"
#include "catapult/model/RangeTypes.h"
#include "catapult/utils/Hashers.h"
#include "catapult/utils/SpinReaderWriterLock.h"
#include <set>
#include <unordered_map>

namespace catapult { namespace cache { struct TransactionData; } }

namespace catapult { namespace cache {

	/// Internal container wrapped by MemoryUtCache.
	/// \note std::set is used to allow incomplete type.
	using TransactionDataContainer = std::set<TransactionData>;

	/// Read only view on top of unconfirmed transactions cache.
	class MemoryUtCacheView {
	private:
		using UnknownTransactions = std::vector<std::shared_ptr<const model::Transaction>>;
		using IdLookup = std::unordered_map<Hash256, size_t, utils::ArrayHasher<Hash256>>;
		using TransactionInfoConsumer = predicate<const model::TransactionInfo&>;

	public:
		/// Creates a view around a maximum response size (\a maxResponseSize), a transaction data container
		/// (\a transactionDataContainer) and an id lookup (\a idLookup) with lock context \a readLock.
		MemoryUtCacheView(
				uint64_t maxResponseSize,
				const TransactionDataContainer& transactionDataContainer,
				const IdLookup& idLookup,
				utils::SpinReaderWriterLock::ReaderLockGuard&& readLock);

	public:
		/// Gets the number of unconfirmed transactions in the cache.
		size_t size() const;

		/// Returns \c true if the cache contains an unconfirmed transaction with associated \a hash, \c false otherwise.
		bool contains(const Hash256& hash) const;

		/// Calls \a consumer with all transaction infos until all are consumed or \c false is returned by consumer.
		void forEach(const TransactionInfoConsumer& consumer) const;

		/// Gets a range of short hashes of all transactions in the cache.
		/// \note Each short hash consists of the first 4 bytes of the complete hash.
		model::ShortHashRange shortHashes() const;

		/// Gets a vector of all transactions in the cache that have a fee multiplier at least \a minFeeMultiplier
		/// and do not have a short hash in \a knownShortHashes.
		UnknownTransactions unknownTransactions(BlockFeeMultiplier minFeeMultiplier, const utils::ShortHashesSet& knownShortHashes) const;

	private:
		uint64_t m_maxResponseSize;
		const TransactionDataContainer& m_transactionDataContainer;
		const IdLookup& m_idLookup;
		utils::SpinReaderWriterLock::ReaderLockGuard m_readLock;
	};

	/// Interface (read write) for caching unconfirmed transactions.
	class PLUGIN_API_DEPENDENCY ReadWriteUtCache : public UtCache {
	public:
		/// Gets a read only view based on this cache.
		virtual MemoryUtCacheView view() const = 0;
	};

	/// Cache for all unconfirmed transactions.
	class MemoryUtCache : public ReadWriteUtCache {
	public:
		using CacheWriteOnlyInterface = UtCache;
		using CacheReadWriteInterface = ReadWriteUtCache;

	public:
		/// Creates an unconfirmed transactions cache around \a options.
		explicit MemoryUtCache(const MemoryCacheOptions& options);

		/// Destroys an unconfirmed transactions cache.
		~MemoryUtCache() override;

	public:
		MemoryUtCacheView view() const override;

		UtCacheModifierProxy modifier() override;

	private:
		struct Impl;

	private:
		MemoryCacheOptions m_options;
		size_t m_idSequence;
		std::unique_ptr<Impl> m_pImpl;
		mutable utils::SpinReaderWriterLock m_lock;
	};

	/// Delegating proxy around a MemoryUtCache.
	class MemoryUtCacheProxy : public MemoryCacheProxy<MemoryUtCache> {
		using MemoryCacheProxy<MemoryUtCache>::MemoryCacheProxy;
	};
}}
