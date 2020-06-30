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
#include "HashCheckOptions.h"
#include "catapult/chain/ChainFunctions.h"
#include "catapult/utils/Hashers.h"
#include "catapult/utils/SpinLock.h"
#include "catapult/types.h"
#include <unordered_map>

namespace catapult { namespace consumers {

	/// Hash cache that holds recently seen hashes.
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

	/// Synchronized wrapper around a RecentHashCache.
	class SynchronizedRecentHashCache {
	public:
		/// Creates a recent hash cache around \a timeSupplier and \a options.
		SynchronizedRecentHashCache(const chain::TimeSupplier& timeSupplier, const HashCheckOptions& options);

	public:
		/// Checks if \a hash is already in the cache and adds it to the cache if it is unknown.
		/// \note This also prunes the hash cache.
		bool add(const Hash256& hash);

	private:
		RecentHashCache m_recentHashCache;
		utils::SpinLock m_lock;
	};
}}
