/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

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
			auto tryLock() {
				return m_detachedDelta.tryLock();
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

		std::unique_ptr<CatapultCacheDelta> getAndTryLock() {
			return m_pLockableUnconfirmedCatapultCache->tryLock();
		}

		std::unique_ptr<CatapultCacheDelta> rebaseAndLock() {
			auto detachableDelta = m_catapultCache.createDetachableDelta();
			m_cacheHeight = detachableDelta.height();
			m_pLockableUnconfirmedCatapultCache = std::make_unique<DetachedDeltaWrapper>(detachableDelta.detach());
			return m_pLockableUnconfirmedCatapultCache->tryLock();
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

	std::unique_ptr<CatapultCacheDelta> RelockableDetachedCatapultCache::getAndTryLock() {
		return m_pImpl->getAndTryLock();
	}

	std::unique_ptr<CatapultCacheDelta> RelockableDetachedCatapultCache::rebaseAndLock() {
		return m_pImpl->rebaseAndLock();
	}
}}
