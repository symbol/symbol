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
