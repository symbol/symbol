#include "RelockableDetachedCatapultCache.h"
#include "CatapultCache.h"
#include "CatapultCacheDetachedDelta.h"

namespace catapult { namespace cache {

	namespace {
		// this wrapper is needed because CatapultCacheDetachedDelta / LockableCacheDelta is not assignable
		// due to a const& member variable
		class DetachedDeltaWrapper {
		public:
			explicit DetachedDeltaWrapper(CatapultCacheDetachedDelta&& detachedDelta)
					: m_detachedDelta(std::move(detachedDelta))
			{}

		public:
			auto lock() {
				return m_detachedDelta.lock();
			}

		private:
			CatapultCacheDetachedDelta m_detachedDelta;
		};
	}

	class RelockableDetachedCatapultCache::Impl final {
	public:
		explicit Impl(const CatapultCache& catapultCache) : m_catapultCache(catapultCache) {
			rebaseAndLock();
		}

	public:
		Height height() const {
			return m_cacheHeight;
		}

		std::unique_ptr<CatapultCacheDelta> getAndLock() {
			return m_pLockableUnconfirmedCatapultCache->lock();
		}

		std::unique_ptr<CatapultCacheDelta> rebaseAndLock() {
			auto detachableDelta = m_catapultCache.createDetachableDelta();
			m_cacheHeight = detachableDelta.height();
			m_pLockableUnconfirmedCatapultCache = std::make_unique<DetachedDeltaWrapper>(detachableDelta.detach());
			return m_pLockableUnconfirmedCatapultCache->lock();
		}

	private:
		const CatapultCache& m_catapultCache;
		Height m_cacheHeight;
		std::unique_ptr<DetachedDeltaWrapper> m_pLockableUnconfirmedCatapultCache;
	};

	RelockableDetachedCatapultCache::RelockableDetachedCatapultCache(const CatapultCache& catapultCache)
			: m_pImpl(std::make_unique<Impl>(catapultCache))
	{}

	RelockableDetachedCatapultCache::~RelockableDetachedCatapultCache() = default;

	Height RelockableDetachedCatapultCache::height() const {
		return m_pImpl->height();
	}

	std::unique_ptr<CatapultCacheDelta> RelockableDetachedCatapultCache::getAndLock() {
		return m_pImpl->getAndLock();
	}

	std::unique_ptr<CatapultCacheDelta> RelockableDetachedCatapultCache::rebaseAndLock() {
		return m_pImpl->rebaseAndLock();
	}
}}
