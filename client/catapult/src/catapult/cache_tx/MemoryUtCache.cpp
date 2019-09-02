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

#include "MemoryUtCache.h"
#include "AccountCounters.h"
#include "CacheSizeLogger.h"
#include "catapult/model/EntityInfo.h"
#include "catapult/model/FeeUtils.h"

namespace catapult { namespace cache {

	struct TransactionData : public model::TransactionInfo, public utils::NonCopyable {
	public:
		explicit TransactionData(size_t id)
				: model::TransactionInfo()
				, Id(id)
		{}

		TransactionData(const model::TransactionInfo& transactionInfo, size_t id)
				: model::TransactionInfo(transactionInfo.copy())
				, Id(id)
		{}

	public:
		bool operator<(const TransactionData& rhs) const {
			return Id < rhs.Id;
		}

	public:
		size_t Id;
	};

	// region MemoryUtCacheView

	MemoryUtCacheView::MemoryUtCacheView(
			uint64_t maxResponseSize,
			const TransactionDataContainer& transactionDataContainer,
			const IdLookup& idLookup,
			utils::SpinReaderWriterLock::ReaderLockGuard&& readLock)
			: m_maxResponseSize(maxResponseSize)
			, m_transactionDataContainer(transactionDataContainer)
			, m_idLookup(idLookup)
			, m_readLock(std::move(readLock))
	{}

	size_t MemoryUtCacheView::size() const {
		return m_transactionDataContainer.size();
	}

	bool MemoryUtCacheView::contains(const Hash256& hash) const {
		return m_idLookup.cend() != m_idLookup.find(hash);
	}

	void MemoryUtCacheView::forEach(const TransactionInfoConsumer& consumer) const {
		for (const auto& data : m_transactionDataContainer) {
			if (!consumer(data))
				return;
		}
	}

	model::ShortHashRange MemoryUtCacheView::shortHashes() const {
		auto shortHashes = model::EntityRange<utils::ShortHash>::PrepareFixed(m_transactionDataContainer.size());
		auto shortHashesIter = shortHashes.begin();
		for (const auto& data : m_transactionDataContainer)
			*shortHashesIter++ = utils::ToShortHash(data.EntityHash);

		return shortHashes;
	}

	MemoryUtCacheView::UnknownTransactions MemoryUtCacheView::unknownTransactions(
			BlockFeeMultiplier minFeeMultiplier,
			const utils::ShortHashesSet& knownShortHashes) const {
		uint64_t totalSize = 0;
		UnknownTransactions transactions;
		for (const auto& data : m_transactionDataContainer) {
			if (data.pEntity->MaxFee < model::CalculateTransactionFee(minFeeMultiplier, *data.pEntity))
				continue;

			auto shortHash = utils::ToShortHash(data.EntityHash);
			auto iter = knownShortHashes.find(shortHash);
			if (knownShortHashes.cend() == iter) {
				auto pTransaction = data.pEntity;
				totalSize += pTransaction->Size;
				if (totalSize > m_maxResponseSize)
					break;

				transactions.push_back(pTransaction);
			}
		}

		return transactions;
	}

	// endregion

	// region MemoryUtCacheModifier

	namespace {
		class MemoryUtCacheModifier : public UtCacheModifier {
		private:
			using IdLookup = std::unordered_map<Hash256, size_t, utils::ArrayHasher<Hash256>>;

		public:
			MemoryUtCacheModifier(
					uint64_t maxCacheSize,
					size_t& idSequence,
					TransactionDataContainer& transactionDataContainer,
					IdLookup& idLookup,
					AccountCounters& counters,
					utils::SpinReaderWriterLock::ReaderLockGuard&& readLock)
					: m_maxCacheSize(maxCacheSize)
					, m_idSequence(idSequence)
					, m_transactionDataContainer(transactionDataContainer)
					, m_idLookup(idLookup)
					, m_counters(counters)
					, m_readLock(std::move(readLock))
					, m_writeLock(m_readLock.promoteToWriter())
			{}

		public:
			size_t size() const override {
				return m_transactionDataContainer.size();
			}

			bool add(const model::TransactionInfo& transactionInfo) override {
				if (m_maxCacheSize <= m_transactionDataContainer.size())
					return false;

				if (m_idLookup.cend() != m_idLookup.find(transactionInfo.EntityHash))
					return false;

				m_idLookup.emplace(transactionInfo.EntityHash, ++m_idSequence);
				m_transactionDataContainer.emplace(transactionInfo, m_idSequence);

				m_counters.increment(transactionInfo.pEntity->SignerPublicKey);

				LogSizes("unconfirmed transactions", m_transactionDataContainer.size(), m_maxCacheSize);
				return true;
			}

			model::TransactionInfo remove(const Hash256& hash) override {
				auto iter = m_idLookup.find(hash);
				if (m_idLookup.cend() == iter)
					return model::TransactionInfo();

				auto dataIter = m_transactionDataContainer.find(TransactionData(iter->second));
				auto erasedInfo = dataIter->copy();

				m_counters.decrement(dataIter->pEntity->SignerPublicKey);

				m_transactionDataContainer.erase(dataIter);
				m_idLookup.erase(iter);
				return erasedInfo;
			}

			size_t count(const Key& key) const override {
				return m_counters.count(key);
			}

			std::vector<model::TransactionInfo> removeAll() override {
				if (!m_transactionDataContainer.empty())
					CATAPULT_LOG(debug) << "removing " << m_transactionDataContainer.size() << " elements from ut cache";

				// unfortunately cannot just move m_transactionDataContainer because it contains a different (derived) type
				std::vector<model::TransactionInfo> transactionInfosCopy;
				transactionInfosCopy.reserve(m_transactionDataContainer.size());

				for (const auto& data : m_transactionDataContainer)
					transactionInfosCopy.emplace_back(data.copy());

				m_transactionDataContainer.clear();
				m_idLookup.clear();
				m_counters.reset();
				return transactionInfosCopy;
			}

		private:
			uint64_t m_maxCacheSize;
			size_t& m_idSequence;
			TransactionDataContainer& m_transactionDataContainer;
			IdLookup& m_idLookup;
			AccountCounters& m_counters;
			utils::SpinReaderWriterLock::ReaderLockGuard m_readLock;
			utils::SpinReaderWriterLock::WriterLockGuard m_writeLock;
		};
	}

	// endregion

	// region MemoryUtCache

	struct MemoryUtCache::Impl {
		cache::TransactionDataContainer TransactionDataContainer;
		std::unordered_map<Hash256, size_t, utils::ArrayHasher<Hash256>> IdLookup;
		AccountCounters Counters;
	};

	MemoryUtCache::MemoryUtCache(const MemoryCacheOptions& options)
			: m_options(options)
			, m_idSequence(0)
			, m_pImpl(std::make_unique<Impl>())
	{}

	MemoryUtCache::~MemoryUtCache() = default;

	MemoryUtCacheView MemoryUtCache::view() const {
		auto readLock = m_lock.acquireReader();
		return MemoryUtCacheView(m_options.MaxResponseSize, m_pImpl->TransactionDataContainer, m_pImpl->IdLookup, std::move(readLock));
	}

	UtCacheModifierProxy MemoryUtCache::modifier() {
		auto readLock = m_lock.acquireReader();
		return UtCacheModifierProxy(std::make_unique<MemoryUtCacheModifier>(
				m_options.MaxCacheSize,
				m_idSequence,
				m_pImpl->TransactionDataContainer,
				m_pImpl->IdLookup,
				m_pImpl->Counters,
				std::move(readLock)));
	}

	// endregion
}}
