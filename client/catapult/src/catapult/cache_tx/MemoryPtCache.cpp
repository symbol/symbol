/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "MemoryPtCache.h"
#include "CacheSizeLogger.h"
#include "catapult/crypto/Hashes.h"
#include "catapult/model/Cosignature.h"
#include "catapult/state/TimestampedHash.h"
#include <set>

namespace catapult { namespace cache {

	// region PtData

	class PtData {
	public:
		explicit PtData(const model::DetachedTransactionInfo& transactionInfo)
				: m_transactionInfo(transactionInfo.copy())
				, m_cosignaturesHash() // zero-initialize
		{}

	public:
		model::DetachedTransactionInfo transactionInfo() const {
			return m_transactionInfo.copy();
		}

		std::shared_ptr<const model::Transaction> transaction() const {
			return m_transactionInfo.pEntity;
		}

		const Hash256& entityHash() const {
			return m_transactionInfo.EntityHash;
		}

		const std::vector<model::Cosignature>& cosignatures() const {
			return m_cosignatures;
		}

		const Hash256& cosignaturesHash() const {
			return m_cosignaturesHash;
		}

		model::WeakCosignedTransactionInfo weakCosignedTransactionInfo() const {
			return { transaction().get(), &m_cosignatures };
		}

		state::TimestampedHash timestampedHash() const {
			return state::TimestampedHash(transaction()->Deadline, entityHash());
		}

	public:
		bool add(const model::Cosignature& cosignature) {
			if (weakCosignedTransactionInfo().hasCosignatory(cosignature.SignerPublicKey))
				return false;

			// insert cosignature into sorted vector
			auto iter = m_cosignatures.begin();
			while (m_cosignatures.end() != iter && iter->SignerPublicKey < cosignature.SignerPublicKey)
				++iter;

			m_cosignatures.insert(iter, cosignature);

			// recalculate the cosignatures hash
			crypto::Sha3_256(
					{ reinterpret_cast<const uint8_t*>(m_cosignatures.data()), m_cosignatures.size() * sizeof(model::Cosignature) },
					m_cosignaturesHash);
			return true;
		}

	private:
		model::DetachedTransactionInfo m_transactionInfo;
		Hash256 m_cosignaturesHash;

		// sorted by SignerPublicKey so that sets of cosignatures added in different order match
		std::vector<model::Cosignature> m_cosignatures;
	};

	// endregion

	// region MemoryPtCacheView

	MemoryPtCacheView::MemoryPtCacheView(
			utils::FileSize maxResponseSize,
			utils::FileSize cacheSize,
			const PtDataContainer& transactionDataContainer,
			utils::SpinReaderWriterLock::ReaderLockGuard&& readLock)
			: m_maxResponseSize(maxResponseSize)
			, m_cacheSize(cacheSize)
			, m_transactionDataContainer(transactionDataContainer)
			, m_readLock(std::move(readLock))
	{}

	size_t MemoryPtCacheView::size() const {
		return m_transactionDataContainer.size();
	}

	utils::FileSize MemoryPtCacheView::memorySize() const {
		return m_cacheSize;
	}

	model::WeakCosignedTransactionInfo MemoryPtCacheView::find(const Hash256& hash) const {
		auto iter = m_transactionDataContainer.find(hash);
		return m_transactionDataContainer.cend() == iter
				? model::WeakCosignedTransactionInfo()
				: iter->second.weakCosignedTransactionInfo();
	}

	ShortHashPairRange MemoryPtCacheView::shortHashPairs() const {
		auto shortHashPairs = model::EntityRange<ShortHashPair>::PrepareFixed(m_transactionDataContainer.size());
		auto shortHashPairsIter = shortHashPairs.begin();
		for (const auto& pair : m_transactionDataContainer) {
			const auto& ptData = pair.second;
			shortHashPairsIter->TransactionShortHash = utils::ToShortHash(ptData.entityHash());
			shortHashPairsIter->CosignaturesShortHash = utils::ToShortHash(ptData.cosignaturesHash());
			++shortHashPairsIter;
		}

		return shortHashPairs;
	}

	MemoryPtCacheView::UnknownTransactionInfos MemoryPtCacheView::unknownTransactions(
			Timestamp minDeadline,
			const ShortHashPairMap& knownShortHashPairs) const {
		uint64_t totalSize = 0;
		UnknownTransactionInfos unknownTransactionInfos;
		for (const auto& pair : m_transactionDataContainer) {
			const auto& ptData = pair.second;
			auto shortHash = utils::ToShortHash(ptData.entityHash());
			auto iter = knownShortHashPairs.find(shortHash);

			// if both hashes match, the data is completely known, so skip it
			if (knownShortHashPairs.cend() != iter && iter->second == utils::ToShortHash(ptData.cosignaturesHash()))
				continue;

			if (ptData.transaction()->Deadline < minDeadline)
				continue;

			auto entrySize = sizeof(Hash256) + sizeof(model::Cosignature) * ptData.cosignatures().size();
			model::CosignedTransactionInfo transactionInfo;
			transactionInfo.EntityHash = ptData.entityHash();
			transactionInfo.Cosignatures = ptData.cosignatures();

			// only add the transaction if it is unknown
			if (knownShortHashPairs.cend() == iter) {
				transactionInfo.pTransaction = ptData.transaction();
				entrySize += transactionInfo.pTransaction->Size;
			}

			totalSize += entrySize;
			if (totalSize > m_maxResponseSize.bytes())
				break;

			unknownTransactionInfos.push_back(transactionInfo);
		}

		return unknownTransactionInfos;
	}

	// endregion

	// region MemoryPtCacheModifier

	namespace {
		model::DetachedTransactionInfo ToTransactionInfo(const PtDataContainer::value_type& pair) {
			return pair.second.transactionInfo();
		}

		class MemoryPtCacheModifier : public PtCacheModifier {
		public:
			MemoryPtCacheModifier(
					utils::FileSize maxCacheSize,
					utils::FileSize& cacheSize,
					PtDataContainer& transactionDataContainer,
					std::set<state::TimestampedHash>& timestampedHashes,
					utils::SpinReaderWriterLock::WriterLockGuard&& writeLock)
					: m_maxCacheSize(maxCacheSize)
					, m_cacheSize(cacheSize)
					, m_transactionDataContainer(transactionDataContainer)
					, m_timestampedHashes(timestampedHashes)
					, m_writeLock(std::move(writeLock))
			{}

		public:
			size_t size() const override {
				return m_transactionDataContainer.size();
			}

			utils::FileSize memorySize() const override {
				return m_cacheSize;
			}

			bool add(const model::DetachedTransactionInfo& transactionInfo) override {
				auto transactionSize = transactionInfo.pEntity->Size;
				if (m_maxCacheSize.bytes() - m_cacheSize.bytes() < transactionSize)
					return false;

				auto iter = m_transactionDataContainer.find(transactionInfo.EntityHash);
				if (m_transactionDataContainer.cend() != iter)
					return false;

				m_transactionDataContainer.emplace(transactionInfo.EntityHash, PtData(transactionInfo));
				m_timestampedHashes.emplace(transactionInfo.pEntity->Deadline, transactionInfo.EntityHash);

				auto oldCacheSize = m_cacheSize;
				m_cacheSize = utils::FileSize::FromBytes(m_cacheSize.bytes() + transactionSize);
				LogSizes("partial transactions", oldCacheSize, m_cacheSize, m_maxCacheSize);
				return true;
			}

			model::DetachedTransactionInfo add(const Hash256& parentHash, const model::Cosignature& cosignature) override {
				auto iter = m_transactionDataContainer.find(parentHash);
				if (m_transactionDataContainer.cend() == iter || !iter->second.add(cosignature))
					return model::DetachedTransactionInfo();

				// don't enforce maxCacheSize here or partials might not be able to complete when cache is full
				m_cacheSize = utils::FileSize::FromBytes(m_cacheSize.bytes() + sizeof(model::Cosignature));
				return ToTransactionInfo(*iter);
			}

			model::DetachedTransactionInfo remove(const Hash256& hash) override {
				auto iter = m_transactionDataContainer.find(hash);
				if (m_transactionDataContainer.cend() == iter)
					return model::DetachedTransactionInfo();

				auto erasedInfo = ToTransactionInfo(*iter);
				remove(iter);
				return erasedInfo;
			}

			std::vector<model::DetachedTransactionInfo> prune(Timestamp timestamp) override {
				std::vector<model::DetachedTransactionInfo> prunedInfos;
				while (!m_timestampedHashes.empty()) {
					auto timestampedHashesIter = m_timestampedHashes.cbegin();
					if (timestampedHashesIter->Time > timestamp)
						break;

					auto iter = m_transactionDataContainer.find(timestampedHashesIter->Hash);
					prunedInfos.push_back(ToTransactionInfo(*iter));
					remove(iter);
				}

				return prunedInfos;
			}

			std::vector<model::DetachedTransactionInfo> prune(const predicate<const Hash256&>& hashPredicate) override {
				std::vector<model::DetachedTransactionInfo> prunedInfos;
				for (auto iter = m_transactionDataContainer.begin(); m_transactionDataContainer.end() != iter;) {
					if (!hashPredicate(iter->second.entityHash())) {
						++iter;
						continue;
					}

					prunedInfos.push_back(ToTransactionInfo(*iter));
					remove(iter++);
				}

				return prunedInfos;
			}

		private:
			void remove(PtDataContainer::iterator iter) {
				m_timestampedHashes.erase(iter->second.timestampedHash());
				auto numRemovedBytes = iter->second.transaction()->Size + sizeof(model::Cosignature) * iter->second.cosignatures().size();
				m_cacheSize = utils::FileSize::FromBytes(m_cacheSize.bytes() - numRemovedBytes);
				m_transactionDataContainer.erase(iter);
			}

		private:
			utils::FileSize m_maxCacheSize;
			utils::FileSize& m_cacheSize;
			PtDataContainer& m_transactionDataContainer;
			std::set<state::TimestampedHash>& m_timestampedHashes;
			utils::SpinReaderWriterLock::WriterLockGuard m_writeLock;
		};
	}

	// endregion

	// region MemoryPtCache

	struct MemoryPtCache::Impl {
		PtDataContainer TransactionDataContainer;
		utils::FileSize CacheSize;

		std::set<state::TimestampedHash> TimestampedHashes;
	};

	MemoryPtCache::MemoryPtCache(const MemoryCacheOptions& options)
			: m_options(options)
			, m_pImpl(std::make_unique<Impl>())
	{}

	MemoryPtCache::~MemoryPtCache() = default;

	MemoryPtCacheView MemoryPtCache::view() const {
		auto readLock = m_lock.acquireReader();
		return MemoryPtCacheView(m_options.MaxResponseSize, m_pImpl->CacheSize, m_pImpl->TransactionDataContainer, std::move(readLock));
	}

	PtCacheModifierProxy MemoryPtCache::modifier() {
		auto writeLock = m_lock.acquireWriter();
		return PtCacheModifierProxy(std::make_unique<MemoryPtCacheModifier>(
				m_options.MaxCacheSize,
				m_pImpl->CacheSize,
				m_pImpl->TransactionDataContainer,
				m_pImpl->TimestampedHashes,
				std::move(writeLock)));
	}

	// endregion
}}
