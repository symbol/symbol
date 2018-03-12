#include "ReadOnlyAccountStateCache.h"
#include "AccountStateCacheDelta.h"
#include "AccountStateCacheView.h"

namespace catapult { namespace cache {

	namespace {
		template<typename TKey>
		auto Contains(const TKey& key, const BasicAccountStateCacheView* pCache, const BasicAccountStateCacheDelta* pCacheDelta) {
			return nullptr != pCache ? pCache->contains(key) : pCacheDelta->contains(key);
		}

		template<typename TKey>
		auto& Get(const TKey& key, const BasicAccountStateCacheView* pCache, const BasicAccountStateCacheDelta* pCacheDelta) {
			return nullptr != pCache ? pCache->get(key) : pCacheDelta->get(key);
		}

		template<typename TKey>
		auto* TryGet(const TKey& key, const BasicAccountStateCacheView* pCache, const BasicAccountStateCacheDelta* pCacheDelta) {
			return nullptr != pCache ? pCache->tryGet(key) : pCacheDelta->tryGet(key);
		}
	}

	model::NetworkIdentifier ReadOnlyAccountStateCache::networkIdentifier() const {
		return nullptr != m_pCache ? m_pCache->networkIdentifier() : m_pCacheDelta->networkIdentifier();
	}

	uint64_t ReadOnlyAccountStateCache::importanceGrouping() const {
		return nullptr != m_pCache ? m_pCache->importanceGrouping() : m_pCacheDelta->importanceGrouping();
	}

	size_t ReadOnlyAccountStateCache::size() const {
		return nullptr != m_pCache ? m_pCache->size() : m_pCacheDelta->size();
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
