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
#include <stddef.h>

namespace catapult { namespace cache {

	/// Read-only overlay on top of a cache that provides support for contains.
	template<typename TCache, typename TCacheDelta, typename TCacheKey>
	class ReadOnlySimpleCache {
	public:
		/// Creates a read-only overlay on top of \a cache.
		explicit ReadOnlySimpleCache(const TCache& cache)
				: m_pCache(&cache)
				, m_pCacheDelta(nullptr)
		{}

		/// Creates a read-only overlay on top of \a cache.
		explicit ReadOnlySimpleCache(const TCacheDelta& cache)
				: m_pCache(nullptr)
				, m_pCacheDelta(&cache)
		{}

	public:
		/// Gets the number of elements in the cache.
		size_t size() const {
			return m_pCache ? m_pCache->size() : m_pCacheDelta->size();
		}

		/// Searches for the given \a key in the cache.
		/// Returns \c true if it is found or \c false otherwise.
		bool contains(const TCacheKey& key) const {
			return m_pCache ? m_pCache->contains(key) : m_pCacheDelta->contains(key);
		}

	private:
		const TCache* m_pCache;
		const TCacheDelta* m_pCacheDelta;
	};
}}
