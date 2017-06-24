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
			return nullptr != m_pCache ? m_pCache->get(id) : m_pCacheDelta->get(id);
		}

		/// Gets a value indicating whether or not an artifact with \a id is active at \a height.
		bool isActive(TKey id, Height height) const {
			return nullptr != m_pCache ? m_pCache->isActive(id, height) : m_pCacheDelta->isActive(id, height);
		}

	private:
		const TCache* m_pCache;
		const TCacheDelta* m_pCacheDelta;
	};
}}
