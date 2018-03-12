#pragma once
#include "CatapultCacheDelta.h"
#include "catapult/types.h"

namespace catapult { namespace cache { class CatapultCache; } }

namespace catapult { namespace cache {

	/// A relockable detached catapult cache.
	class RelockableDetachedCatapultCache {
	public:
		/// Creates a relockable detached catapult cache around \a catapultCache.
		explicit RelockableDetachedCatapultCache(const CatapultCache& catapultCache);

		/// Destroys the relockable detached catapult cache.
		~RelockableDetachedCatapultCache();

	public:
		/// Gets the current cache height.
		Height height() const;

		/// Gets and locks the last (detached) catapult cache delta.
		/// \note If locking fails, \c nullptr is returned.
		std::unique_ptr<CatapultCacheDelta> getAndLock();

		/// Rebases and locks the (detached) catapult cache delta.
		std::unique_ptr<CatapultCacheDelta> rebaseAndLock();

	private:
		class Impl;
		std::unique_ptr<Impl> m_pImpl;
	};
}}
