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
#include "BlockDifficultyCache.h"
#include "BlockDifficultyCacheStorage.h"
#include "catapult/cache/SubCachePluginAdapter.h"

namespace catapult { namespace cache {

	/// CacheStorage implementation for saving and loading summary block difficulty cache data.
	class BlockDifficultyCacheSummaryCacheStorage : public CacheStorageAdapter<BlockDifficultyCache, BlockDifficultyCacheStorage> {
	public:
		using CacheStorageAdapter<BlockDifficultyCache, BlockDifficultyCacheStorage>::CacheStorageAdapter;

	public:
		void saveSummary(const CatapultCacheDelta& cacheDelta, io::OutputStream& output) const override;
	};

	/// Specialized block difficulty cache sub cache plugin.
	class BlockDifficultyCacheSubCachePlugin : public SubCachePluginAdapter<BlockDifficultyCache, BlockDifficultyCacheStorage> {
	public:
		/// Creates a plugin around \a difficultyHistorySize.
		explicit BlockDifficultyCacheSubCachePlugin(uint64_t difficultyHistorySize);

	public:
		std::unique_ptr<CacheStorage> createStorage() override;
	};
}}
