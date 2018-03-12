#pragma once
#include "CatapultCacheDelta.h"
#include <vector>

namespace catapult { namespace cache { class DetachedSubCacheView; } }

namespace catapult { namespace cache {

	/// Detached delta of the catapult cache.
	class CatapultCacheDetachedDelta {
	public:
		/// Creates a detached cache delta from \a detachedSubViews.
		explicit CatapultCacheDetachedDelta(std::vector<std::unique_ptr<DetachedSubCacheView>>&& detachedSubViews);

		/// Destroys the delta.
		~CatapultCacheDetachedDelta();

	public:
		// make this class move only (the definitions are in the source file in order to allow forward declarations)
		CatapultCacheDetachedDelta(CatapultCacheDetachedDelta&&);
		CatapultCacheDetachedDelta& operator=(CatapultCacheDetachedDelta&&);

	public:
		/// Locks the detached delta.
		/// \note If locking fails, \c nullptr is returned.
		std::unique_ptr<CatapultCacheDelta> lock();

	private:
		std::vector<std::unique_ptr<DetachedSubCacheView>> m_detachedSubViews;
	};
}}
