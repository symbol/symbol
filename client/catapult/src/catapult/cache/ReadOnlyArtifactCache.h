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
#include "ReadOnlySimpleCache.h"
#include "catapult/types.h"

namespace catapult { namespace cache {

	/// A read-only overlay on top of a cache that provides support for contains, get and isActive.
	template<typename TCache, typename TCacheDelta, typename TKey, typename TGetResult>
	class ReadOnlyArtifactCache : public ReadOnlySimpleCache<TCache, TCacheDelta, TKey> {
	public:
		/// Creates a read-only overlay on top of \a cache.
		explicit ReadOnlyArtifactCache(const TCache& cache)
				: ReadOnlySimpleCache<TCache, TCacheDelta, TKey>(cache)
				, m_pCache(&cache)
				, m_pCacheDelta(nullptr)
		{}

		/// Creates a read-only overlay on top of \a cache.
		explicit ReadOnlyArtifactCache(const TCacheDelta& cache)
				: ReadOnlySimpleCache<TCache, TCacheDelta, TKey>(cache)
				, m_pCache(nullptr)
				, m_pCacheDelta(&cache)
		{}

	public:
		/// Gets an entry specified by its \a id.
		/// \note The method will throw if the id is unknown.
		TGetResult get(TKey id) const {
			return m_pCache ? m_pCache->get(id) : m_pCacheDelta->get(id);
		}

		/// Gets a value indicating whether or not an artifact with \a id is active at \a height.
		bool isActive(TKey id, Height height) const {
			return m_pCache ? m_pCache->isActive(id, height) : m_pCacheDelta->isActive(id, height);
		}

	private:
		const TCache* m_pCache;
		const TCacheDelta* m_pCacheDelta;
	};
}}
