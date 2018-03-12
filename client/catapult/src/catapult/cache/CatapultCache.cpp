#include "CatapultCache.h"
#include "CacheHeight.h"
#include "CatapultCacheDetachedDelta.h"
#include "ReadOnlyCatapultCache.h"
#include "SubCachePluginAdapter.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/model/NetworkInfo.h"

namespace catapult { namespace cache {

	namespace {
		template<typename TSubCacheViews>
		std::vector<const void*> ExtractReadOnlyViews(const TSubCacheViews& subViews) {
			std::vector<const void*> readOnlyViews;
			for (const auto& pSubView : subViews) {
				if (!pSubView) {
					readOnlyViews.push_back(nullptr);
					continue;
				}

				readOnlyViews.push_back(pSubView->asReadOnly());
			}

			return readOnlyViews;
		}
	}

	// region CatapultCacheView

	CatapultCacheView::CatapultCacheView(CacheHeightView&& cacheHeightView, std::vector<std::unique_ptr<const SubCacheView>>&& subViews)
			: m_pCacheHeight(std::make_unique<CacheHeightView>(std::move(cacheHeightView)))
			, m_subViews(std::move(subViews))
	{}

	CatapultCacheView::~CatapultCacheView() = default;

	CatapultCacheView::CatapultCacheView(CatapultCacheView&&) = default;

	CatapultCacheView& CatapultCacheView::operator=(CatapultCacheView&&) = default;

	Height CatapultCacheView::height() const {
		return m_pCacheHeight->get();
	}

	ReadOnlyCatapultCache CatapultCacheView::toReadOnly() const {
		return ReadOnlyCatapultCache(ExtractReadOnlyViews(m_subViews));
	}

	// endregion

	// region CatapultCacheDelta

	CatapultCacheDelta::CatapultCacheDelta(std::vector<std::unique_ptr<SubCacheView>>&& subViews)
			: m_subViews(std::move(subViews))
	{}

	CatapultCacheDelta::~CatapultCacheDelta() = default;

	CatapultCacheDelta::CatapultCacheDelta(CatapultCacheDelta&&) = default;

	CatapultCacheDelta& CatapultCacheDelta::operator=(CatapultCacheDelta&&) = default;

	ReadOnlyCatapultCache CatapultCacheDelta::toReadOnly() const {
		return ReadOnlyCatapultCache(ExtractReadOnlyViews(m_subViews));
	}

	// endregion

	// region CatapultCacheDetachableDelta

	CatapultCacheDetachableDelta::CatapultCacheDetachableDelta(
			CacheHeightView&& cacheHeightView,
			std::vector<std::unique_ptr<DetachedSubCacheView>>&& detachedSubViews)
			// note that CacheHeightView is a unique_ptr to allow CatapultCacheDetachableDelta to be declared without it defined
			: m_pCacheHeightView(std::make_unique<CacheHeightView>(std::move(cacheHeightView)))
			, m_detachedDelta(std::move(detachedSubViews))
	{}

	CatapultCacheDetachableDelta::~CatapultCacheDetachableDelta() = default;

	CatapultCacheDetachableDelta::CatapultCacheDetachableDelta(CatapultCacheDetachableDelta&&) = default;

	Height CatapultCacheDetachableDelta::height() const {
		return m_pCacheHeightView->get();
	}

	CatapultCacheDetachedDelta CatapultCacheDetachableDelta::detach() {
		return std::move(m_detachedDelta);
	}

	// endregion

	// region CatapultCacheDetachedDelta

	CatapultCacheDetachedDelta::CatapultCacheDetachedDelta(std::vector<std::unique_ptr<DetachedSubCacheView>>&& detachedSubViews)
			: m_detachedSubViews(std::move(detachedSubViews))
	{}

	CatapultCacheDetachedDelta::~CatapultCacheDetachedDelta() = default;

	CatapultCacheDetachedDelta::CatapultCacheDetachedDelta(CatapultCacheDetachedDelta&&) = default;

	CatapultCacheDetachedDelta& CatapultCacheDetachedDelta::operator=(CatapultCacheDetachedDelta&&) = default;

	std::unique_ptr<CatapultCacheDelta> CatapultCacheDetachedDelta::lock() {
		std::vector<std::unique_ptr<SubCacheView>> subViews;
		for (const auto& pDetachedSubView : m_detachedSubViews) {
			if (!pDetachedSubView) {
				subViews.push_back(nullptr);
				continue;
			}

			auto pSubView = pDetachedSubView->lock();
			if (!pSubView)
				return nullptr;

			subViews.push_back(std::move(pSubView));
		}

		return std::make_unique<CatapultCacheDelta>(std::move(subViews));
	}

	// endregion

	// region CatapultCache

	namespace {
		template<typename TResultView, typename TSubCaches, typename TMapper>
		std::vector<std::unique_ptr<TResultView>> MapSubCaches(TSubCaches& subCaches, TMapper map, bool includeNulls = true) {
			std::vector<std::unique_ptr<TResultView>> resultViews;
			for (const auto& pSubCache : subCaches) {
				if (!pSubCache && !includeNulls)
					continue;

				resultViews.push_back(pSubCache ? map(pSubCache) : nullptr);
			}

			return resultViews;
		}
	}

	CatapultCache::CatapultCache(std::vector<std::unique_ptr<SubCachePlugin>>&& subCaches)
			: m_pCacheHeight(std::make_unique<CacheHeight>())
			, m_subCaches(std::move(subCaches))
	{}

	CatapultCache::~CatapultCache() = default;

	CatapultCache::CatapultCache(CatapultCache&&) = default;

	CatapultCache& CatapultCache::operator=(CatapultCache&&) = default;

	CatapultCacheView CatapultCache::createView() const {
		// acquire a height reader lock to ensure the view is composed of consistent subcache views
		auto pCacheHeightView = m_pCacheHeight->view();
		auto subViews = MapSubCaches<const SubCacheView>(m_subCaches, [](const auto& pSubCache) { return pSubCache->createView(); });
		return CatapultCacheView(std::move(pCacheHeightView), std::move(subViews));
	}

	CatapultCacheDelta CatapultCache::createDelta() {
		// since only one subcache delta is allowed outstanding at a time and an outstanding delta is required for commit,
		// subcache deltas will always be consistent
		auto subViews = MapSubCaches<SubCacheView>(m_subCaches, [](const auto& pSubCache) { return pSubCache->createDelta(); });
		return CatapultCacheDelta(std::move(subViews));
	}

	CatapultCacheDetachableDelta CatapultCache::createDetachableDelta() const {
		// acquire a height reader lock to ensure the delta is composed of consistent subcache deltas
		auto pCacheHeightView = m_pCacheHeight->view();
		auto detachedSubViews = MapSubCaches<DetachedSubCacheView>(m_subCaches, [](const auto& pSubCache) {
			return pSubCache->createDetachedDelta();
		});
		return CatapultCacheDetachableDelta(std::move(pCacheHeightView), std::move(detachedSubViews));
	}

	void CatapultCache::commit(Height height) {
		// use the height writer lock to lock the entire cache during commit
		auto cacheHeightModifier = m_pCacheHeight->modifier();

		for (const auto& pSubCache : m_subCaches) {
			if (pSubCache)
				pSubCache->commit();
		}

		// finally, update the cache height
		cacheHeightModifier.set(height);
	}

	std::vector<std::unique_ptr<const CacheStorage>> CatapultCache::storages() const {
		return MapSubCaches<const CacheStorage>(
				m_subCaches,
				[this](const auto& pSubCache) { return pSubCache->createStorage(*this); },
				false);
	}

	std::vector<std::unique_ptr<CacheStorage>> CatapultCache::storages() {
		return MapSubCaches<CacheStorage>(
				m_subCaches,
				[this](const auto& pSubCache) { return pSubCache->createStorage(*this); },
				false);
	}

	// endregion
}}
