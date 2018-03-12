#include "RecentHashCache.h"
#include "catapult/utils/ContainerHelpers.h"
#include "catapult/utils/Logging.h"

namespace catapult { namespace consumers {

	RecentHashCache::RecentHashCache(const chain::TimeSupplier& timeSupplier, const HashCheckOptions& options)
			: m_timeSupplier(timeSupplier)
			, m_options(options)
			, m_lastPruneTime(m_timeSupplier())
	{}

	size_t RecentHashCache::size() const {
		return m_cache.size();
	}

	bool RecentHashCache::add(const Hash256& hash) {
		auto currentTime = m_timeSupplier();
		auto isHashKnown = checkAndUpdateExisting(hash, currentTime);
		pruneCache(currentTime);

		if (!isHashKnown)
			tryAddToCache(hash, currentTime);

		return !isHashKnown;
	}

	bool RecentHashCache::contains(const Hash256& hash) const {
		return m_cache.cend() != m_cache.find(hash);
	}

	bool RecentHashCache::checkAndUpdateExisting(const Hash256& hash, const Timestamp& time) {
		auto iter = m_cache.find(hash);
		if (m_cache.end() != iter) {
			iter->second = time;
			return true;
		}

		return false;
	}

	void RecentHashCache::pruneCache(const Timestamp& time) {
		if (time - m_lastPruneTime < Timestamp(m_options.PruneInterval))
			return;

		m_lastPruneTime = time;
		utils::map_erase_if(m_cache, [duration = m_options.CacheDuration, time](const auto& pair) {
			return pair.second + Timestamp(duration) < time;
		});
	}

	void RecentHashCache::tryAddToCache(const Hash256& hash, const Timestamp& time) {
		// only add the hash if the cache is not full
		if (m_options.MaxCacheSize <= m_cache.size())
			return;

		m_cache.emplace(hash, time);
		if (m_options.MaxCacheSize == m_cache.size())
			CATAPULT_LOG(warning) << "short lived hash check cache is full";
	}
}}
