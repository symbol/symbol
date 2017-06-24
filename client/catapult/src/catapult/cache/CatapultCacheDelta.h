#pragma once
#include "SubCachePlugin.h"
#include <memory>
#include <vector>

namespace catapult { namespace cache { class ReadOnlyCatapultCache; } }

namespace catapult { namespace cache {
	/// Delta on top of a catapult cache.
	class CatapultCacheDelta {
	public:
		/// Creates a locked catapult cache delta from \a subViews.
		explicit CatapultCacheDelta(std::vector<std::unique_ptr<SubCacheView>>&& subViews);

		/// Destroys the delta.
		~CatapultCacheDelta();

	public:
		// make this class move only (the definitions are in the source file in order to allow forward declarations)
		CatapultCacheDelta(CatapultCacheDelta&&);
		CatapultCacheDelta& operator=(CatapultCacheDelta&&);

	public:
		/// Gets a specific subcache delta view.
		template<typename TCache>
		const typename TCache::CacheDeltaType& sub() const {
			return *static_cast<const typename TCache::CacheDeltaType*>(m_subViews[TCache::Id]->get());
		}

		/// Gets a specific subcache delta view.
		template<typename TCache>
		typename TCache::CacheDeltaType& sub() {
			return *static_cast<typename TCache::CacheDeltaType*>(m_subViews[TCache::Id]->get());
		}

	public:
		/// Creates a read-only view of this delta.
		ReadOnlyCatapultCache toReadOnly() const;

	private:
		std::vector<std::unique_ptr<SubCacheView>> m_subViews;
	};
}}
