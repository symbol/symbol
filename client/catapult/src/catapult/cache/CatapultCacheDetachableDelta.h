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
		/// Creates a detachable cache delta from a cache height view (\a cacheHeightView) and \a detachedSubViews.
		CatapultCacheDetachableDelta(
				CacheHeightView&& cacheHeightView,
				std::vector<std::unique_ptr<DetachedSubCacheView>>&& detachedSubViews);

		/// Destroys the detachable cache delta.
		~CatapultCacheDetachableDelta();

	public:
		// Move assignment operator
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
