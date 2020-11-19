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
#include <vector>

namespace catapult { namespace cache { class DetachedSubCacheView; } }

namespace catapult { namespace cache {

	/// Detached delta of the catapult cache.
	class CatapultCacheDetachedDelta {
	public:
		/// Creates a detached cache delta from \a dependentState and \a detachedSubViews.
		CatapultCacheDetachedDelta(
				const state::CatapultState& dependentState,
				std::vector<std::unique_ptr<DetachedSubCacheView>>&& detachedSubViews);

		/// Destroys the delta.
		~CatapultCacheDetachedDelta();

	public:
		// make this class move only (the definitions are in the source file in order to allow forward declarations)
		CatapultCacheDetachedDelta(CatapultCacheDetachedDelta&&);
		CatapultCacheDetachedDelta& operator=(CatapultCacheDetachedDelta&&);

	public:
		/// Locks the detached delta.
		/// \note If locking fails, \c nullptr is returned.
		std::unique_ptr<CatapultCacheDelta> tryLock();

	private:
		std::unique_ptr<state::CatapultState> m_pDependentState;
		std::vector<std::unique_ptr<DetachedSubCacheView>> m_detachedSubViews;
	};
}}
