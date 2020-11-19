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
#include "CatapultCacheDetachedDelta.h"
#include "catapult/types.h"

namespace catapult { namespace cache { class CacheHeightView; } }

namespace catapult { namespace cache {

	/// Detachable delta of the catapult cache.
	/// \note This delta acquires a reader lock (via the cache height view) to the catapult cache that is released
	///       when the delta is destroyed.
	class CatapultCacheDetachableDelta {
	public:
		/// Creates a detachable cache delta from a cache height view (\a cacheHeightView), \a dependentState and \a detachedSubViews.
		CatapultCacheDetachableDelta(
				CacheHeightView&& cacheHeightView,
				const state::CatapultState& dependentState,
				std::vector<std::unique_ptr<DetachedSubCacheView>>&& detachedSubViews);

		/// Destroys the detachable cache delta.
		~CatapultCacheDetachableDelta();

	public:
		/// Move constructor.
		CatapultCacheDetachableDelta(CatapultCacheDetachableDelta&&);

	public:
		/// Gets the cache height associated with this delta.
		Height height() const;

		/// Detaches the detached delta from this delta.
		CatapultCacheDetachedDelta detach();

	private:
		std::unique_ptr<const CacheHeightView> m_pCacheHeightView;
		CatapultCacheDetachedDelta m_detachedDelta;
	};
}}
