#pragma once
#include "catapult/cache/CatapultCache.h"
#include <functional>

namespace catapult { namespace mongo { namespace plugins {

	/// Interface for loading and saving cache data to external storage.
	class ExternalCacheStorage {
	public:
		virtual ~ExternalCacheStorage() {}

	public:
		/// Gets the cache name.
		virtual const std::string& name() const = 0;

		/// Gets the cache id.
		virtual size_t id() const = 0;

	public:
		/// Saves \a cache delta data to external storage.
		virtual void saveDelta(const cache::CatapultCacheDelta& cache) = 0;

		/// Loads data from external storage into \a cache given the current chain height (\a chainHeight).
		virtual void loadAll(cache::CatapultCache& cache, Height chainHeight) const = 0;
	};

	/// Typed interface for loading and saving cache data to external storage.
	template<typename TCache>
	class ExternalCacheStorageT : public ExternalCacheStorage {
	protected:
		/// Load checkpoint function.
		using LoadCheckpointFunc = std::function<void ()>;

	public:
		/// Creates an external cache storage.
		ExternalCacheStorageT() : m_name(TCache::Name)
		{}

	public:
		const std::string& name() const final override {
			return m_name;
		}

		size_t id() const final override {
			return TCache::Id;
		}

	public:
		void saveDelta(const cache::CatapultCacheDelta& cache) final override {
			saveDelta(cache.sub<TCache>());
		}

		void loadAll(cache::CatapultCache& cache, Height chainHeight) const final override {
			auto delta = cache.createDelta();
			LoadCheckpointFunc checkpoint = [&cache, chainHeight]() { cache.commit(chainHeight); };
			loadAll(delta.sub<TCache>(), chainHeight, checkpoint);
			checkpoint();
		}

	private:
		/// Saves \a cache delta data to external storage.
		virtual void saveDelta(const typename TCache::CacheDeltaType& cache) = 0;

		/// Loads data from external storage into \a cache given the current chain height (\a chainHeight)
		/// with optional checkpoints created by calling \a checkpoint.
		virtual void loadAll(typename TCache::CacheDeltaType& cache, Height chainHeight, const LoadCheckpointFunc& checkpoint) const = 0;

	private:
		std::string m_name;
	};
}}}
