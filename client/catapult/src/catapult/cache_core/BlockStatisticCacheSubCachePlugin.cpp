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

#include "BlockStatisticCacheSubCachePlugin.h"

namespace catapult { namespace cache {

	namespace {
		class BlockStatisticCacheSummaryCacheStorage : public CacheStorageAdapter<BlockStatisticCache, BlockStatisticCacheStorage> {
		public:
			using CacheStorageAdapter<BlockStatisticCache, BlockStatisticCacheStorage>::CacheStorageAdapter;

		public:
			void saveSummary(const CatapultCacheDelta& cacheDelta, io::OutputStream& output) const override {
				const auto& delta = cacheDelta.sub<BlockStatisticCache>();
				io::Write64(output, delta.size());

				auto pIterableView = delta.tryMakeIterableView();
				for (const auto& value : *pIterableView)
					BlockStatisticCacheStorage::Save(value, output);

				output.flush();
			}
		};
	}

	BlockStatisticCacheSubCachePlugin::BlockStatisticCacheSubCachePlugin(uint64_t historySize)
			: SubCachePluginAdapter<BlockStatisticCache, BlockStatisticCacheStorage>(std::make_unique<BlockStatisticCache>(historySize))
	{}

	std::unique_ptr<CacheStorage> BlockStatisticCacheSubCachePlugin::createStorage() {
		return std::make_unique<BlockStatisticCacheSummaryCacheStorage>(cache());
	}
}}
