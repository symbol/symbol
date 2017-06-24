#include "AccountStateCacheView.h"
#include "catapult/model/Address.h"
#include "catapult/model/NetworkInfo.h"

namespace catapult { namespace cache {
	using ConstAccountStatePointer = account_state_cache_types::ConstPointerType;

	bool BasicAccountStateCacheView::contains(const Address& address) const {
		return m_stateByAddress.contains(address);
	}

	bool BasicAccountStateCacheView::contains(const Key& publicKey) const {
		auto pPair = m_keyToAddress.find(publicKey);
		return pPair && contains(pPair->second);
	}

	ConstAccountStatePointer BasicAccountStateCacheView::findAccount(const Address& address) const {
		return m_stateByAddress.find(address);
	}

	ConstAccountStatePointer BasicAccountStateCacheView::findAccount(const Key& publicKey) const {
		auto pPair = m_keyToAddress.find(publicKey);
		return pPair ? findAccount(pPair->second) : nullptr;
	}
}}
