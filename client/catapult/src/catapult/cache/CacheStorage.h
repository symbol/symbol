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
#include "CacheStorageInclude.h"
#include "catapult/plugins.h"
#include <string>

namespace catapult {
	namespace cache {
		class CatapultCacheDelta;
		class CatapultCacheView;
	}
}

namespace catapult { namespace cache {

	/// Interface for loading and saving cache data.
	class PLUGIN_API_DEPENDENCY CacheStorage {
	public:
		virtual ~CacheStorage() = default;

	public:
		/// Gets the cache name.
		virtual const std::string& name() const = 0;

	public:
		/// Saves cache data from \a cacheView to \a output.
		virtual void saveAll(const CatapultCacheView& cacheView, io::OutputStream& output) const = 0;

		/// Saves cache (summary) data from \a cacheDelta to \a output.
		virtual void saveSummary(const CatapultCacheDelta& cacheDelta, io::OutputStream& output) const = 0;

		/// Loads cache data from \a input in batches of \a batchSize.
		virtual void loadAll(io::InputStream& input, size_t batchSize) = 0;
	};
}}
