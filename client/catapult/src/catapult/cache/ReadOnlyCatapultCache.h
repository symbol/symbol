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
#include <vector>

namespace catapult { namespace state { struct CatapultState; } }

namespace catapult { namespace cache {

	/// Read-only overlay on top of a catapult cache.
	class ReadOnlyCatapultCache {
	public:
		/// Creates a read-only overlay on top of \a dependentState and \a readOnlyViews.
		ReadOnlyCatapultCache(const state::CatapultState& dependentState, const std::vector<const void*>& readOnlyViews);

	public:
		/// Gets a specific sub cache read-only view.
		template<typename TCache>
		const typename TCache::CacheReadOnlyType& sub() const {
			return *static_cast<const typename TCache::CacheReadOnlyType*>(m_readOnlyViews[TCache::Id]);
		}

	public:
		/// Gets the (const) dependent catapult state.
		const state::CatapultState& dependentState() const;

	private:
		const state::CatapultState& m_dependentState;
		std::vector<const void*> m_readOnlyViews;
	};
}}
