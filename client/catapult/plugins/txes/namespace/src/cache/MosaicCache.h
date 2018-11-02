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
#include "MosaicCacheDelta.h"
#include "MosaicCacheStorage.h"
#include "MosaicCacheView.h"
#include "catapult/cache/BasicCache.h"

namespace catapult { namespace cache {

	// size_t needs to be const& so that view and delta are created with latest value
	// but since it is a POD, they can and do store it by value
	using MosaicBasicCache = BasicCache<MosaicCacheDescriptor, MosaicCacheTypes::BaseSets, const size_t&>;

	/// Cache composed of mosaic information.
	class BasicMosaicCache : public MosaicBasicCache {
	public:
		/// Creates a cache around \a config.
		explicit BasicMosaicCache(const CacheConfiguration& config) : BasicMosaicCache(config, std::make_unique<size_t>())
		{}

	private:
		BasicMosaicCache(const CacheConfiguration& config, std::unique_ptr<size_t>&& pDeepSize)
				: MosaicBasicCache(config, *pDeepSize)
				, m_pDeepSize(std::move(pDeepSize))
		{}

	public:
		/// Initializes the cache with \a deepSize.
		void init(size_t deepSize) {
			*m_pDeepSize = deepSize;
		}

		/// Commits all pending changes to the underlying storage.
		/// \note This hides MosaicBasicCache::commit.
		void commit(const CacheDeltaType& delta) {
			MosaicBasicCache::commit(delta);
			*m_pDeepSize = delta.deepSize();
		}

	private:
		// unique pointer to allow reference to be valid after moves of this cache
		std::unique_ptr<size_t> m_pDeepSize;
	};

	/// Synchronized cache composed of mosaic information.
	class MosaicCache : public SynchronizedCacheWithInit<BasicMosaicCache> {
	public:
		DEFINE_CACHE_CONSTANTS(Mosaic)

	public:
		/// Creates a cache around \a config.
		explicit MosaicCache(const CacheConfiguration& config) : SynchronizedCacheWithInit<BasicMosaicCache>(BasicMosaicCache(config))
		{}
	};
}}
