#include "MemoryUtCache.h"
#include "CacheSizeLogger.h"
#include "catapult/model/EntityInfo.h"

namespace catapult { namespace cache {

	struct TransactionData : public model::TransactionInfo, public utils::NonCopyable {
	public:
		explicit TransactionData(const model::TransactionInfo& transactionInfo, size_t id)
				: model::TransactionInfo(transactionInfo.copy())
				, Id(id)
		{}

		explicit TransactionData(size_t id)
				: model::TransactionInfo()
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

	MemoryUtCacheView::UnknownTransactions MemoryUtCacheView::unknownTransactions(const utils::ShortHashesSet& knownShortHashes) const {
		uint64_t totalSize = 0;
		UnknownTransactions transactions;
		for (const auto& data : m_transactionDataContainer) {
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
			explicit MemoryUtCacheModifier(
					uint64_t maxCacheSize,
					size_t& idSequence,
					TransactionDataContainer& transactionDataContainer,
					IdLookup& idLookup,
					utils::SpinReaderWriterLock::ReaderLockGuard&& readLock)
					: m_maxCacheSize(maxCacheSize)
					, m_idSequence(idSequence)
					, m_transactionDataContainer(transactionDataContainer)
					, m_idLookup(idLookup)
					, m_readLock(std::move(readLock))
					, m_writeLock(m_readLock.promoteToWriter())
			{}

		public:
			bool add(const model::TransactionInfo& transactionInfo) override {
				if (m_maxCacheSize <= m_transactionDataContainer.size())
					return false;

				if (m_idLookup.cend() != m_idLookup.find(transactionInfo.EntityHash))
					return false;

				m_idLookup.emplace(transactionInfo.EntityHash, ++m_idSequence);
				m_transactionDataContainer.emplace(transactionInfo, m_idSequence);

				LogSizes("unconfirmed transactions", m_transactionDataContainer.size(), m_maxCacheSize);
				return true;
			}

			model::TransactionInfo remove(const Hash256& hash) override {
				auto iter = m_idLookup.find(hash);
				if (m_idLookup.cend() == iter)
					return model::TransactionInfo();

				auto dataIter = m_transactionDataContainer.find(TransactionData(iter->second));
				auto erasedInfo = dataIter->copy();

				m_transactionDataContainer.erase(dataIter);
				m_idLookup.erase(iter);
				return erasedInfo;
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
				return transactionInfosCopy;
			}

		private:
			uint64_t m_maxCacheSize;
			size_t& m_idSequence;
			TransactionDataContainer& m_transactionDataContainer;
			IdLookup& m_idLookup;
			utils::SpinReaderWriterLock::ReaderLockGuard m_readLock;
			utils::SpinReaderWriterLock::WriterLockGuard m_writeLock;
		};
	}

	// endregion

	// region MemoryUtCache

	struct MemoryUtCache::Impl {
		cache::TransactionDataContainer TransactionDataContainer;
		std::unordered_map<Hash256, size_t, utils::ArrayHasher<Hash256>> IdLookup;
	};

	MemoryUtCache::MemoryUtCache(const MemoryCacheOptions& options)
			: m_options(options)
			, m_idSequence(0)
			, m_pImpl(std::make_unique<Impl>())
	{}

	MemoryUtCache::~MemoryUtCache() = default;

	MemoryUtCacheView MemoryUtCache::view() const {
		return MemoryUtCacheView(m_options.MaxResponseSize, m_pImpl->TransactionDataContainer, m_pImpl->IdLookup, m_lock.acquireReader());
	}

	UtCacheModifierProxy MemoryUtCache::modifier() {
		return UtCacheModifierProxy(std::make_unique<MemoryUtCacheModifier>(
				m_options.MaxCacheSize,
				m_idSequence,
				m_pImpl->TransactionDataContainer,
				m_pImpl->IdLookup,
				m_lock.acquireReader()));
	}

	// endregion
}}
