#pragma once
#include "catapult/cache/CatapultCache.h"
#include "catapult/functions.h"

namespace catapult { namespace mongo {

	/// Abstract class for loading and saving cache data to external storage.
	class ExternalCacheStorage {
	protected:
		/// Creates an external cache storage around \a name and \a id.
		ExternalCacheStorage(const std::string& name, size_t id)
				: m_name(name)
				, m_id(id)
		{}

	public:
		virtual ~ExternalCacheStorage() {}

	public:
		/// Gets the cache name.
		const std::string& name() const {
			return m_name;
		}

		/// Gets the cache id.
		size_t id() const {
			return m_id;
		}

	public:
		/// Saves \a cache delta data to external storage.
		virtual void saveDelta(const cache::CatapultCacheDelta& cache) = 0;

		/// Loads data from external storage into \a cache given the current chain height (\a chainHeight).
		virtual void loadAll(cache::CatapultCache& cache, Height chainHeight) const = 0;

	private:
		std::string m_name;
		size_t m_id;
	};

	/// Typed interface for loading and saving cache data to external storage.
	template<typename TCache>
	class ExternalCacheStorageT : public ExternalCacheStorage {
	protected:
		/// Load checkpoint function.
		using LoadCheckpointFunc = action;

	public:
		/// Creates an external cache storage.
		ExternalCacheStorageT() : ExternalCacheStorage(TCache::Name, TCache::Id)
		{}

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
	};
}}
