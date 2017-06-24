#pragma once
#include <stddef.h>

namespace catapult { namespace cache {

	/// A read-only overlay on top of a cache that provides support for contains.
	template<typename TCache, typename TCacheDelta, typename TKey>
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
		/// Returns the number of elements in the cache.
		size_t size() const {
			return nullptr != m_pCache ? m_pCache->size() : m_pCacheDelta->size();
		}

		/// Searches for the given \a key in the cache.
		/// Returns \c true if it is found or \c false otherwise.
		bool contains(const TKey& key) const {
			return nullptr != m_pCache ? m_pCache->contains(key) : m_pCacheDelta->contains(key);
		}

	private:
		const TCache* m_pCache;
		const TCacheDelta* m_pCacheDelta;
	};
}}
