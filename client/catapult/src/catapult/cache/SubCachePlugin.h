#pragma once
#include <memory>

namespace catapult {
	namespace cache {
		class CacheStorage;
		class CatapultCache;
	}
}

namespace catapult { namespace cache {

	/// A subcache view.
	class SubCacheView {
	public:
		virtual ~SubCacheView() {}

	public:
		/// Returns a const pointer to the underlying view.
		virtual const void* get() const = 0;

		/// Returns a pointer to the underlying view.
		virtual void* get() = 0;

		/// Returns a read-only view of this view.
		virtual const void* asReadOnly() const = 0;
	};

	/// A detached subcache view.
	class DetachedSubCacheView {
	public:
		virtual ~DetachedSubCacheView() {}

	public:
		/// Locks the cache delta.
		/// \note Returns \c nullptr if the detached delta is no longer valid.
		virtual std::unique_ptr<SubCacheView> lock() = 0;
	};

	/// A subcache plugin that can be added to the main catapult cache.
	class SubCachePlugin {
	public:
		virtual ~SubCachePlugin() {}

	public:
		/// Returns a locked cache view based on this cache.
		virtual std::unique_ptr<const SubCacheView> createView() const = 0;

		/// Returns a locked cache delta based on this cache.
		/// \note Changes to an attached delta can be committed by calling commit.
		virtual std::unique_ptr<SubCacheView> createDelta() = 0;

		/// Returns a lockable cache delta based on this cache but without the ability
		/// to commit any changes to the original cache.
		virtual std::unique_ptr<DetachedSubCacheView> createDetachedDelta() const = 0;

		/// Commits all pending changes to the underlying storage.
		virtual void commit() = 0;

	public:
		/// Returns a const pointer to the underlying cache.
		virtual const void* get() const = 0;

	public:
		/// Returns a cache storage based on this cache given \a catapultCache, which provides current cache state
		/// and allows the establishment of links between subcaches.
		virtual std::unique_ptr<CacheStorage> createStorage(const cache::CatapultCache& catapultCache) = 0;
	};
}}
