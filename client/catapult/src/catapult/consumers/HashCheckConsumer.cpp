#include "BlockConsumers.h"
#include "ConsumerResultFactory.h"
#include "TransactionConsumers.h"
#include "catapult/utils/Hashers.h"
#include <unordered_map>

namespace catapult { namespace consumers {

	namespace {
		using TimeGenerator = std::function<Timestamp ()>;

		class RecentHashCache {
		public:
			RecentHashCache(const TimeGenerator& timeGenerator, const HashCheckOptions& options)
					: m_timeGenerator(timeGenerator)
					, m_options(options)
					, m_lastPruneTime(m_timeGenerator())
			{}

		public:
			bool add(const Hash256& hash) {
				auto currentTime = m_timeGenerator();
				auto isHashKnown = checkAndUpdateExisting(hash, currentTime);
				pruneCache(currentTime);

				if (!isHashKnown)
					tryAddToCache(hash, currentTime);

				return !isHashKnown;
			}

		private:
			bool checkAndUpdateExisting(const Hash256& hash, const Timestamp& time) {
				auto iter = m_cache.find(hash);
				if (m_cache.end() != iter) {
					iter->second = time;
					return true;
				}

				return false;
			}

			void pruneCache(const Timestamp& time) {
				if (time - m_lastPruneTime < Timestamp(m_options.PruneInterval))
					return;

				m_lastPruneTime = time;
				for (auto iter = m_cache.begin(); m_cache.end() != iter; ) {
					if (iter->second + Timestamp(m_options.CacheDuration) < time)
						iter = m_cache.erase(iter);
					else
						++iter;
				}
			}

			void tryAddToCache(const Hash256& hash, const Timestamp& time) {
				// only add the hash if the cache is not full
				if (m_options.MaxCacheSize <= m_cache.size())
					return;

				m_cache.emplace(hash, time);
				if (m_options.MaxCacheSize == m_cache.size())
					CATAPULT_LOG(warning) << "short lived hash check cache is full";
			}

		private:
			TimeGenerator m_timeGenerator;
			HashCheckOptions m_options;
			Timestamp m_lastPruneTime;
			std::unordered_map<Hash256, Timestamp, utils::ArrayHasher<Hash256>> m_cache;
		};

		class BlockHashCheckConsumer {
		public:
			BlockHashCheckConsumer(const TimeGenerator& timeGenerator, const HashCheckOptions& options)
					: m_recentHashCache(timeGenerator, options)
			{}

		public:
			ConsumerResult operator()(const BlockElements& elements) {
				if (elements.empty())
					return Abort(Failure_Consumer_Empty_Input);

				// only check block hashes when processing single elements
				if (1 != elements.size())
					return Continue();

				return m_recentHashCache.add(elements[0].EntityHash)
						? Continue()
						: Abort(Failure_Consumer_Hash_In_Recency_Cache);
			}

		private:
			RecentHashCache m_recentHashCache;
		};
	}

	disruptor::ConstBlockConsumer CreateBlockHashCheckConsumer(
			const TimeGenerator& timeGenerator,
			const HashCheckOptions& options) {
		return BlockHashCheckConsumer(timeGenerator, options);
	}

	namespace {
		class TransactionHashCheckConsumer {
		public:
			TransactionHashCheckConsumer(
					const TimeGenerator& timeGenerator,
					const HashCheckOptions& options,
					const KnownHashPredicate& knownHashPredicate)
					: m_recentHashCache(timeGenerator, options)
					, m_knownHashPredicate(knownHashPredicate)
			{}

		public:
			ConsumerResult operator()(TransactionElements& elements) {
				if (elements.empty())
					return Abort(Failure_Consumer_Empty_Input);

				auto numSkipped = 0u;
				for (auto& element : elements) {
					if (!shouldSkip(element))
						continue;

					// already seen
					element.Skip = true;
					++numSkipped;
				}

				if (elements.size() != numSkipped)
					return Continue();

				CATAPULT_LOG(trace) << "all " << numSkipped << " transaction(s) skipped in TransactionHashCheck, skipping whole element";
				return Abort(Failure_Consumer_Hash_In_Recency_Cache);
			}

		private:
			bool shouldSkip(const model::TransactionElement& element) {
				if (!m_recentHashCache.add(element.EntityHash))
					return true;

				return m_knownHashPredicate(element.Transaction.Deadline, element.EntityHash);
			}

		private:
			RecentHashCache m_recentHashCache;
			KnownHashPredicate m_knownHashPredicate;
		};
	}

	disruptor::TransactionConsumer CreateTransactionHashCheckConsumer(
			const TimeGenerator& timeGenerator,
			const HashCheckOptions& options,
			const KnownHashPredicate& knownHashPredicate) {
		return TransactionHashCheckConsumer(timeGenerator, options, knownHashPredicate);
	}
}}
