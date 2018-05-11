/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#include "ReadOnlyAccountStateCache.h"
#include "AccountStateCacheDelta.h"
#include "AccountStateCacheView.h"

namespace catapult { namespace cache {

	namespace {
		template<typename TKey>
		auto Contains(const TKey& key, const BasicAccountStateCacheView* pCache, const BasicAccountStateCacheDelta* pCacheDelta) {
			return pCache ? pCache->contains(key) : pCacheDelta->contains(key);
		}

		template<typename TKey>
		auto& Get(const TKey& key, const BasicAccountStateCacheView* pCache, const BasicAccountStateCacheDelta* pCacheDelta) {
			return pCache ? pCache->get(key) : pCacheDelta->get(key);
		}

		template<typename TKey>
		auto* TryGet(const TKey& key, const BasicAccountStateCacheView* pCache, const BasicAccountStateCacheDelta* pCacheDelta) {
			return pCache ? pCache->tryGet(key) : pCacheDelta->tryGet(key);
		}
	}

	model::NetworkIdentifier ReadOnlyAccountStateCache::networkIdentifier() const {
		return m_pCache ? m_pCache->networkIdentifier() : m_pCacheDelta->networkIdentifier();
	}

	uint64_t ReadOnlyAccountStateCache::importanceGrouping() const {
		return m_pCache ? m_pCache->importanceGrouping() : m_pCacheDelta->importanceGrouping();
	}

	size_t ReadOnlyAccountStateCache::size() const {
		return m_pCache ? m_pCache->size() : m_pCacheDelta->size();
	}

	bool ReadOnlyAccountStateCache::contains(const Address& address) const {
		return Contains(address, m_pCache, m_pCacheDelta);
	}

	bool ReadOnlyAccountStateCache::contains(const Key& publicKey) const {
		return Contains(publicKey, m_pCache, m_pCacheDelta);
	}

	const state::AccountState& ReadOnlyAccountStateCache::get(const Address& address) const {
		return Get(address, m_pCache, m_pCacheDelta);
	}

	const state::AccountState& ReadOnlyAccountStateCache::get(const Key& publicKey) const {
		return Get(publicKey, m_pCache, m_pCacheDelta);
	}

	const state::AccountState* ReadOnlyAccountStateCache::tryGet(const Address& address) const {
		return TryGet(address, m_pCache, m_pCacheDelta);
	}

	const state::AccountState* ReadOnlyAccountStateCache::tryGet(const Key& publicKey) const {
		return TryGet(publicKey, m_pCache, m_pCacheDelta);
	}
}}
