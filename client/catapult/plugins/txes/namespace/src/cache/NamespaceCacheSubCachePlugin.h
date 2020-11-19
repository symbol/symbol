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

#pragma once
#include "NamespaceCache.h"
#include "NamespaceCacheStorage.h"
#include "catapult/cache/SummaryAwareSubCachePluginAdapter.h"

namespace catapult { namespace cache {

	/// CacheStorage implementation for saving and loading summary namespace cache data.
	class NamespaceCacheSummaryCacheStorage : public SummaryCacheStorage<NamespaceCache> {
	public:
		using SummaryCacheStorage<NamespaceCache>::SummaryCacheStorage;

	public:
		void saveAll(const CatapultCacheView& cacheView, io::OutputStream& output) const override;

		void saveSummary(const CatapultCacheDelta& cacheDelta, io::OutputStream& output) const override;

		void loadAll(io::InputStream& input, size_t) override;
	};

	using BaseNamespaceCacheSubCachePlugin =
		SummaryAwareSubCachePluginAdapter<NamespaceCache, NamespaceCacheStorage, NamespaceCacheSummaryCacheStorage>;

	/// Specialized namespace cache sub cache plugin.
	class NamespaceCacheSubCachePlugin : public BaseNamespaceCacheSubCachePlugin {
	public:
		/// Creates a plugin around \a config and \a options.
		NamespaceCacheSubCachePlugin(const CacheConfiguration& config, const NamespaceCacheTypes::Options& options);
	};
}}
