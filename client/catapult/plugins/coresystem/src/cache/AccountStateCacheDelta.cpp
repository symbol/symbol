#include "AccountStateCacheDelta.h"
#include "catapult/model/Address.h"
#include "catapult/utils/HexFormatter.h"

namespace catapult { namespace cache {
	using AccountStatePointer = account_state_cache_types::PointerType;
	using ConstAccountStatePointer = account_state_cache_types::ConstPointerType;

	Address BasicAccountStateCacheDelta::getAddress(const Key& publicKey) {
		auto pPair = m_pKeyToAddress->find(publicKey);
		if (pPair)
			return pPair->second;

		auto address = model::PublicKeyToAddress(publicKey, m_networkIdentifier);
		m_pKeyToAddress->emplace(publicKey, address);
		return address;
	}

	AccountStatePointer BasicAccountStateCacheDelta::addAccount(const Address& address, Height height) {
		auto pCurrentState = findAccountOrNull(address);
		if (pCurrentState)
			return pCurrentState;

		auto pAccountState = std::make_shared<state::AccountState>(address, height);
		m_pStateByAddress->insert(pAccountState);
		return pAccountState;
	}

	AccountStatePointer BasicAccountStateCacheDelta::addAccount(const Key& publicKey, Height height) {
		auto address = getAddress(publicKey);
		auto pAccountState = addAccount(address, height);
		if (Height(0) == pAccountState->PublicKeyHeight) {
			pAccountState->PublicKey = publicKey;
			pAccountState->PublicKeyHeight = height;
		}

		return pAccountState;
	}

	AccountStatePointer BasicAccountStateCacheDelta::addAccount(const model::AccountInfo& accountInfo) {
		auto pCurrentState = findAccountOrNull(accountInfo.Address);
		if (pCurrentState)
			return pCurrentState;

		auto pAccountState = std::make_shared<state::AccountState>(accountInfo);
		if (Height(0) != pAccountState->PublicKeyHeight)
			m_pKeyToAddress->emplace(pAccountState->PublicKey, pAccountState->Address);

		m_pStateByAddress->insert(pAccountState);
		return pAccountState;
	}

	bool BasicAccountStateCacheDelta::contains(const Address& address) const {
		return m_pStateByAddress->contains(address);
	}

	bool BasicAccountStateCacheDelta::contains(const Key& publicKey) const {
		auto pPair = m_pKeyToAddress->find(publicKey);
		return pPair && contains(pPair->second);
	}

	namespace {
		template<typename TKey>
		AccountStatePointer ThrowIfNull(AccountStatePointer&& pState, const TKey& key) {
			if (!pState)
				CATAPULT_THROW_INVALID_ARGUMENT_1("inconsistent state, account not present in cache", utils::HexFormat(key));

			return pState;
		}
	}

	AccountStatePointer BasicAccountStateCacheDelta::findAccount(const Address& address) {
		return ThrowIfNull(findAccountOrNull(address), address);
	}

	AccountStatePointer BasicAccountStateCacheDelta::findAccount(const Key& publicKey) {
		return ThrowIfNull(findAccountOrNull(publicKey), publicKey);
	}

	ConstAccountStatePointer BasicAccountStateCacheDelta::findAccount(const Address& address) const {
		return utils::as_const(*m_pStateByAddress).find(address);
	}

	ConstAccountStatePointer BasicAccountStateCacheDelta::findAccount(const Key& publicKey) const {
		auto pPair = utils::as_const(*m_pKeyToAddress).find(publicKey);
		return pPair ? findAccount(pPair->second) : nullptr;
	}

	AccountStatePointer BasicAccountStateCacheDelta::findAccountOrNull(const Address& address) {
		return m_pStateByAddress->find(address);
	}

	AccountStatePointer BasicAccountStateCacheDelta::findAccountOrNull(const Key& publicKey) {
		auto pPair = m_pKeyToAddress->find(publicKey);
		return pPair ? findAccount(pPair->second) : nullptr;
	}

	void BasicAccountStateCacheDelta::remove(const Address& address, Height height) {
		auto pAccountState = findAccountOrNull(address);
		if (!pAccountState || height != pAccountState->AddressHeight)
			return;

		// note: we can only remove the entry from m_pKeyToAddress if the account state's public key is valid
		if (Height(0) != pAccountState->PublicKeyHeight)
			m_pKeyToAddress->remove(pAccountState->PublicKey);

		m_pStateByAddress->remove(address);
	}

	void BasicAccountStateCacheDelta::remove(const Key& publicKey, Height height) {
		auto pAccountState = findAccountOrNull(publicKey);
		if (!pAccountState || height != pAccountState->PublicKeyHeight)
			return;

		m_pKeyToAddress->remove(pAccountState->PublicKey);

		// if same height, remove address entry too
		if (pAccountState->PublicKeyHeight == pAccountState->AddressHeight) {
			m_pStateByAddress->remove(pAccountState->Address);
			return;
		}

		// safe, as the account is still in m_pStateByAddress
		pAccountState->PublicKeyHeight = Height(0);
		pAccountState->PublicKey = Key{};
	}

	void BasicAccountStateCacheDelta::commitRemovals() {
		for (const auto& addressHeightPair : m_queuedRemoveByAddress)
			remove(addressHeightPair.second, addressHeightPair.first);

		for (const auto& keyHeightPair : m_queuedRemoveByPublicKey)
			remove(keyHeightPair.second, keyHeightPair.first);

		m_queuedRemoveByAddress.clear();
		m_queuedRemoveByPublicKey.clear();
	}

	namespace {
		template<typename TDestination, typename TSource>
		void CopyAll(TDestination& dest, const TSource& source) {
			for (const auto& pair : source)
				dest.insert(pair.second);
		}
	}

	BasicAccountStateCacheDelta::AccountStates BasicAccountStateCacheDelta::modifiedAccountStates() const {
		auto deltas = m_pStateByAddress->deltas();

		AccountStates states;
		CopyAll(states, deltas.Added);
		CopyAll(states, deltas.Copied);
		return states;
	}

	BasicAccountStateCacheDelta::AccountStates BasicAccountStateCacheDelta::removedAccountStates() const {
		auto deltas = m_pStateByAddress->deltas();

		AccountStates states;
		CopyAll(states, deltas.Removed);
		return states;
	}
}}
