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
#include "NamespaceCacheDelta.h"
#include "NamespaceCacheStorage.h"
#include "NamespaceCacheView.h"
#include "catapult/cache/BasicCache.h"

namespace catapult { namespace cache {

	using NamespaceBasicCache = BasicCache<
		NamespaceCacheDescriptor,
		NamespaceCacheTypes::BaseSets,
		NamespaceCacheTypes::Options,
		const NamespaceSizes&>;

	/// Cache composed of namespace information.
	class BasicNamespaceCache : public NamespaceBasicCache {
	public:
		/// Creates a cache around \a config and \a options.
		BasicNamespaceCache(const CacheConfiguration& config, const NamespaceCacheTypes::Options& options)
				: BasicNamespaceCache(config, options, std::make_unique<NamespaceSizes>())
		{}

	private:
		BasicNamespaceCache(
				const CacheConfiguration& config,
				const NamespaceCacheTypes::Options& options,
				std::unique_ptr<NamespaceSizes>&& pSizes)
				: NamespaceBasicCache(config, NamespaceCacheTypes::Options(options), *pSizes)
				, m_pSizes(std::move(pSizes))
		{}

	public:
		/// Initializes the cache with \a activeSize and \a deepSize.
		void init(size_t activeSize, size_t deepSize) {
			*m_pSizes = { activeSize, deepSize };
		}

		/// Commits all pending changes to the underlying storage.
		/// \note This hides NamespaceBasicCache::commit.
		void commit(const CacheDeltaType& delta) {
			NamespaceBasicCache::commit(delta);
			*m_pSizes = { delta.activeSize(), delta.deepSize() };
		}

	private:
		// unique pointer to allow reference to be valid after moves of this cache
		std::unique_ptr<NamespaceSizes> m_pSizes;
	};

	/// Synchronized cache composed of namespace information.
	class NamespaceCache : public SynchronizedCacheWithInit<BasicNamespaceCache> {
	public:
		DEFINE_CACHE_CONSTANTS(Namespace)

	public:
		/// Creates a cache around \a config and options.
		NamespaceCache(const CacheConfiguration& config, const NamespaceCacheTypes::Options& options)
				: SynchronizedCacheWithInit<BasicNamespaceCache>(BasicNamespaceCache(config, options))
		{}
	};
}}
