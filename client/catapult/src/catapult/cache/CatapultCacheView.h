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
#include "SubCachePlugin.h"
#include "catapult/types.h"
#include <memory>
#include <vector>

namespace catapult {
	namespace cache {
		class CacheHeightView;
		class ReadOnlyCatapultCache;
	}
}

namespace catapult { namespace cache {

	/// Locked view on top of the catapult cache.
	class CatapultCacheView {
	public:
		/// Creates a locked catapult cache view from a cache height view (\a cacheHeightView) and \a subViews.
		CatapultCacheView(CacheHeightView&& cacheHeightView, std::vector<std::unique_ptr<const SubCacheView>>&& subViews);

		/// Releases the read lock.
		~CatapultCacheView();

	public:
		// make this class move only (the definitions are in the source file in order to allow forward declarations)
		CatapultCacheView(CatapultCacheView&&);
		CatapultCacheView& operator=(CatapultCacheView&&);

	public:
		/// Gets a specific subcache view.
		template<typename TCache>
		const typename TCache::CacheViewType& sub() const {
			return *static_cast<const typename TCache::CacheViewType*>(m_subViews[TCache::Id]->get());
		}

	public:
		/// Gets the cache height associated with the read lock.
		Height height() const;

	public:
		/// Creates a read-only view of this view.
		ReadOnlyCatapultCache toReadOnly() const;

	private:
		std::unique_ptr<CacheHeightView> m_pCacheHeight; // use a unique_ptr to allow fwd declare
		std::vector<std::unique_ptr<const SubCacheView>> m_subViews;
	};
}}
