#include "MemoryUtCache.h"
#include "catapult/model/EntityInfo.h"

namespace catapult { namespace cache {

	struct TransactionData : public model::TransactionInfo, public utils::NonCopyable {
	public:
		explicit TransactionData(model::TransactionInfo&& transactionInfo, size_t id)
				: model::TransactionInfo(std::move(transactionInfo))
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
			const std::set<TransactionData>& transactionDataSet,
			const IdLookup& idLookup,
			utils::SpinReaderWriterLock::ReaderLockGuard&& readLock)
			: m_maxResponseSize(maxResponseSize)
			, m_transactionDataSet(transactionDataSet)
			, m_idLookup(idLookup)
			, m_readLock(std::move(readLock))
	{}

	size_t MemoryUtCacheView::size() const {
		return m_transactionDataSet.size();
	}

	bool MemoryUtCacheView::contains(const Hash256& hash) const {
		return m_idLookup.cend() != m_idLookup.find(hash);
	}

	void MemoryUtCacheView::forEach(const TransactionInfoConsumer& consumer) const {
		for (const auto& data : m_transactionDataSet) {
			if (!consumer(data))
				return;
		}
	}

	namespace {
		utils::ShortHash ToShortHash(const Hash256& hash) {
			return reinterpret_cast<const utils::ShortHash&>(*hash.data());
		}
	}

	model::ShortHashRange MemoryUtCacheView::shortHashes() const {
		auto shortHashes = model::EntityRange<utils::ShortHash>::PrepareFixed(m_transactionDataSet.size());
		auto shortHashesIter = shortHashes.begin();
		for (const auto& data : m_transactionDataSet)
			*shortHashesIter++ = ToShortHash(data.EntityHash);

		return shortHashes;
	}

	MemoryUtCacheView::UnknownTransactions MemoryUtCacheView::unknownTransactions(
			const utils::ShortHashesSet& knownShortHashes) const {
		UnknownTransactions transactions;
		uint64_t totalSize = 0;
		for (const auto& data : m_transactionDataSet) {
			auto shortHash = ToShortHash(data.EntityHash);
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
		void LogCacheSizeIf(size_t actual, uint64_t desired, const char* description) {
			if (actual != desired)
				return;

			CATAPULT_LOG(warning) << "unconfirmed transactions cache is " << description << " (size = " << actual << ")";
		}

		class MemoryUtCacheModifier : public UtCacheModifier {
		private:
			using IdLookup = std::unordered_map<Hash256, size_t, utils::ArrayHasher<Hash256>>;

		public:
			explicit MemoryUtCacheModifier(
					uint64_t maxCacheSize,
					size_t& idSequence,
					std::set<TransactionData>& transactionDataSet,
					IdLookup& idLookup,
					utils::SpinReaderWriterLock::ReaderLockGuard&& readLock)
					: m_maxCacheSize(maxCacheSize)
					, m_idSequence(idSequence)
					, m_transactionDataSet(transactionDataSet)
					, m_idLookup(idLookup)
					, m_readLock(std::move(readLock))
					, m_writeLock(m_readLock.promoteToWriter())
			{}

		public:
			bool add(model::TransactionInfo&& transactionInfo) override {
				if (m_maxCacheSize <= m_transactionDataSet.size())
					return false;

				if (m_idLookup.cend() != m_idLookup.find(transactionInfo.EntityHash))
					return false;

				m_idLookup.emplace(transactionInfo.EntityHash, ++m_idSequence);
				m_transactionDataSet.emplace(std::move(transactionInfo), m_idSequence);

				LogCacheSizeIf(m_transactionDataSet.size(), m_maxCacheSize / 2, "half full");
				LogCacheSizeIf(m_transactionDataSet.size(), m_maxCacheSize, "full");
				return true;
			}

			void remove(const Hash256& hash) override {
				auto iter = m_idLookup.find(hash);
				if (m_idLookup.cend() == iter)
					return;

				m_transactionDataSet.erase(TransactionData(iter->second));
				m_idLookup.erase(iter);
			}

			std::vector<model::TransactionInfo> removeAll() override {
				CATAPULT_LOG(debug) << "UT Cache: " << m_transactionDataSet.size() << " elements";

				// unfortunately cannot just move m_transactionDataSet because it contains a different (derived) type
				std::vector<model::TransactionInfo> copiedInfos;
				copiedInfos.reserve(m_transactionDataSet.size());

				for (auto& data : m_transactionDataSet)
					copiedInfos.emplace_back(data.copy());

				m_transactionDataSet.clear();
				m_idLookup.clear();
				return copiedInfos;
			}

			void prune(Timestamp timestamp) override {
				for (auto iter = m_transactionDataSet.begin(); m_transactionDataSet.end() != iter;) {
					const auto& info = *iter;
					if (timestamp > info.pEntity->Deadline) {
						m_idLookup.erase(info.EntityHash);
						iter = m_transactionDataSet.erase(iter);
						continue;
					}

					++iter;
				}
			}

		private:
			uint64_t m_maxCacheSize;
			size_t& m_idSequence;
			std::set<TransactionData>& m_transactionDataSet;
			IdLookup& m_idLookup;
			utils::SpinReaderWriterLock::ReaderLockGuard m_readLock;
			utils::SpinReaderWriterLock::WriterLockGuard m_writeLock;
		};
	}

	// endregion

	// region MemoryUtCache

	struct MemoryUtCache::Impl {
		std::set<TransactionData> TransactionDataSet;
		std::unordered_map<Hash256, size_t, utils::ArrayHasher<Hash256>> IdLookup;
	};

	MemoryUtCache::MemoryUtCache(const MemoryUtCacheOptions& options)
			: m_options(options)
			, m_idSequence(0)
			, m_pImpl(std::make_unique<Impl>())
	{}

	MemoryUtCache::~MemoryUtCache() = default;

	MemoryUtCacheView MemoryUtCache::view() const {
		return MemoryUtCacheView(
				m_options.MaxResponseSize,
				m_pImpl->TransactionDataSet,
				m_pImpl->IdLookup,
				m_lock.acquireReader());
	}

	UtCacheModifierProxy MemoryUtCache::modifier() {
		return UtCacheModifierProxy(std::make_unique<MemoryUtCacheModifier>(
				m_options.MaxCacheSize,
				m_idSequence,
				m_pImpl->TransactionDataSet,
				m_pImpl->IdLookup,
				m_lock.acquireReader()));
	}

	// endregion
}}
