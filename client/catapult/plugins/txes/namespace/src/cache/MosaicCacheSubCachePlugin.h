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
#include "MosaicCache.h"
#include "MosaicCacheStorage.h"
#include "catapult/cache/SummaryAwareSubCachePluginAdapter.h"

namespace catapult { namespace cache {

	/// CacheStorage implementation for saving and loading summary mosaic cache data.
	class MosaicCacheSummaryCacheStorage : public SummaryCacheStorage<MosaicCache> {
	public:
		using SummaryCacheStorage<MosaicCache>::SummaryCacheStorage;

	public:
		void saveAll(io::OutputStream& output) const override;

		void loadAll(io::InputStream& input, size_t) override;
	};

	using BaseMosaicCacheSubCachePlugin =
		SummaryAwareSubCachePluginAdapter<MosaicCache, MosaicCacheStorage, MosaicCacheSummaryCacheStorage>;

	/// Specialized mosaic cache sub cache plugin.
	class MosaicCacheSubCachePlugin : public BaseMosaicCacheSubCachePlugin {
	public:
		/// Creates a plugin around \a config.
		explicit MosaicCacheSubCachePlugin(const CacheConfiguration& config);
	};
}}
