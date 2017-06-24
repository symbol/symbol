#pragma once
#include <vector>

namespace catapult { namespace cache {

	/// A read-only overlay on top of a catapult cache.
	class ReadOnlyCatapultCache {
	public:
		/// Creates a read-only overlay on top of \a readOnlyViews.
		explicit ReadOnlyCatapultCache(const std::vector<const void*>& readOnlyViews);

	public:
		/// Gets a specific subcache read-only view.
		template<typename TCache>
		const typename TCache::CacheReadOnlyType& sub() const {
			return *static_cast<const typename TCache::CacheReadOnlyType*>(m_readOnlyViews[TCache::Id]);
		}

	private:
		std::vector<const void*> m_readOnlyViews;
	};
}}
