#pragma once
#include "CacheStorageAdapter.h"
#include "SubCachePlugin.h"
#include <memory>

namespace catapult { namespace cache {

	/// A SubCachePlugin implementation that wraps a SynchronizedCache.
	template<typename TCache, typename TStorageTraits>
	class SubCachePluginAdapter : public SubCachePlugin {
	public:
		/// Creates an adapter around \a pCache.
		explicit SubCachePluginAdapter(std::unique_ptr<TCache>&& pCache) : m_pCache(std::move(pCache))
		{}

	public:
		std::unique_ptr<const SubCacheView> createView() const override {
			return std::make_unique<const SubCacheViewAdapter<decltype(m_pCache->createView())>>(m_pCache->createView());
		}

		std::unique_ptr<SubCacheView> createDelta() override {
			return std::make_unique<SubCacheViewAdapter<decltype(m_pCache->createDelta())>>(m_pCache->createDelta());
		}

		std::unique_ptr<DetachedSubCacheView> createDetachedDelta() const override {
			using ViewAdapter = DetachedSubCacheViewAdapter<decltype(m_pCache->createDetachedDelta())>;
			return std::make_unique<ViewAdapter>(m_pCache->createDetachedDelta());
		}

		void commit() override {
			m_pCache->commit();
		}

	public:
		const void* get() const override {
			return m_pCache.get();
		}

	public:
		std::unique_ptr<CacheStorage> createStorage(const cache::CatapultCache& catapultCache) override {
			return std::make_unique<CacheStorageAdapter<TCache, TStorageTraits>>(*m_pCache, catapultCache);
		}

	private:
		template<typename TView>
		class SubCacheViewAdapter : public SubCacheView {
		public:
			explicit SubCacheViewAdapter(TView&& view) : m_view(std::move(view))
			{}

		public:
			const void* get() const override {
				return &*m_view;
			}

			void* get() override {
				return &*m_view;
			}

			const void* asReadOnly() const override {
				return &m_view->asReadOnly();
			}

		private:
			TView m_view;
		};

		template<typename TLockableCacheDelta>
		class DetachedSubCacheViewAdapter : public DetachedSubCacheView {
		public:
			explicit DetachedSubCacheViewAdapter(TLockableCacheDelta&& lockableCacheDelta)
					: m_lockableCacheDelta(std::move(lockableCacheDelta))
			{}

		public:
			std::unique_ptr<SubCacheView> lock() {
				auto delta = m_lockableCacheDelta.lock();
				return delta ? std::make_unique<SubCacheViewAdapter<decltype(delta)>>(std::move(delta)) : nullptr;
			}

		private:
			TLockableCacheDelta m_lockableCacheDelta;
		};

	private:
		std::unique_ptr<TCache> m_pCache;
	};
}}
