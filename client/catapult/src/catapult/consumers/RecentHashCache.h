#pragma once
#include "HashCheckOptions.h"
#include "catapult/chain/ChainFunctions.h"
#include "catapult/utils/Hashers.h"
#include "catapult/types.h"
#include <unordered_map>

namespace catapult { namespace consumers {

	/// A hash cache that holds recently seen hashes.
	class RecentHashCache {
	public:
		/// Creates a recent hash cache around \a timeSupplier and \a options.
		RecentHashCache(const chain::TimeSupplier& timeSupplier, const HashCheckOptions& options);

	public:
		/// Gets the size of the cache.
		size_t size() const;

	public:
		/// Checks if \a hash is already in the cache and adds it to the cache if it is unknown.
		/// \note This also prunes the hash cache.
		bool add(const Hash256& hash);

		/// Returns \c true if the cache contains \a hash, \c false otherwise.
		bool contains(const Hash256& hash) const;

	private:
		bool checkAndUpdateExisting(const Hash256& hash, const Timestamp& time);

		void pruneCache(const Timestamp& time);

		void tryAddToCache(const Hash256& hash, const Timestamp& time);

	private:
		chain::TimeSupplier m_timeSupplier;
		HashCheckOptions m_options;
		Timestamp m_lastPruneTime;
		std::unordered_map<Hash256, Timestamp, utils::ArrayHasher<Hash256>> m_cache;
	};
}}
