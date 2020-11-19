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
#include "CatapultCacheDelta.h"
#include "CatapultCacheDetachableDelta.h"
#include "CatapultCacheView.h"
#include "SubCachePlugin.h"

namespace catapult {
	namespace cache {
		class CacheChangesStorage;
		class CacheHeight;
		class CacheStorage;
		class SubCachePlugin;
	}
	namespace model { struct BlockChainConfiguration; }
}

namespace catapult { namespace cache {

	/// Central cache holding all sub caches.
	class CatapultCache {
	public:
		/// Creates a catapult cache around \a subCaches.
		explicit CatapultCache(std::vector<std::unique_ptr<SubCachePlugin>>&& subCaches);

		/// Destroys the cache.
		~CatapultCache();

	public:
		// make this class move only (the definitions are in the source file in order to allow forward declarations)
		CatapultCache(CatapultCache&&);
		CatapultCache& operator=(CatapultCache&&);

	public:
		/// Gets a specific sub cache.
		template<typename TCache>
		const TCache& sub() const {
			return *static_cast<const TCache*>(m_subCaches[TCache::Id]->get());
		}

	public:
		/// Gets a locked cache view based on this cache.
		CatapultCacheView createView() const;

		/// Gets a locked cache delta based on this cache.
		CatapultCacheDelta createDelta();

		/// Gets a detachable cache delta based on this cache but without the ability
		/// to commit any changes to the original cache.
		/// \note The detachable delta holds a cache reader lock.
		CatapultCacheDetachableDelta createDetachableDelta() const;

		/// Commits all pending changes to the underlying storage and sets the cache height to \a height.
		void commit(Height height);

	public:
		/// Gets the (const) cache storages for all sub caches.
		std::vector<std::unique_ptr<const CacheStorage>> storages() const;

		/// Gets the cache storages for all sub caches.
		std::vector<std::unique_ptr<CacheStorage>> storages();

		/// Gets the (const) cache changes storages for all sub caches.
		std::vector<std::unique_ptr<const CacheChangesStorage>> changesStorages() const;

	private:
		std::unique_ptr<CacheHeight> m_pCacheHeight; // use a unique_ptr to allow fwd declare
		std::unique_ptr<state::CatapultState> m_pDependentState; // use a unique_ptr to allow fwd declare
		std::unique_ptr<state::CatapultState> m_pDependentStateDelta; // backing for (single) outstanding delta
		std::vector<std::unique_ptr<SubCachePlugin>> m_subCaches;
	};
}}
