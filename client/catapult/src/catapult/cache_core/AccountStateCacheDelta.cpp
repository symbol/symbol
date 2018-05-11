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

#include "AccountStateCacheDelta.h"
#include "catapult/model/Address.h"
#include "catapult/state/AccountStateAdapter.h"
#include "catapult/utils/Casting.h"
#include "catapult/utils/HexFormatter.h"
#include "catapult/functions.h"

namespace catapult { namespace cache {

	BasicAccountStateCacheDelta::BasicAccountStateCacheDelta(
			const AccountStateCacheTypes::BaseSetDeltaPointers& accountStateSets,
			const AccountStateCacheTypes::Options& options,
			const model::AddressSet& highValueAddresses)
			: BasicAccountStateCacheDelta(
					accountStateSets,
					options,
					highValueAddresses,
					std::make_unique<AccountStateCacheDeltaMixins::KeyLookupAdapter>(
							*accountStateSets.pKeyLookupMap,
							*accountStateSets.pPrimary))
	{}

	BasicAccountStateCacheDelta::BasicAccountStateCacheDelta(
			const AccountStateCacheTypes::BaseSetDeltaPointers& accountStateSets,
			const AccountStateCacheTypes::Options& options,
			const model::AddressSet& highValueAddresses,
			std::unique_ptr<AccountStateCacheDeltaMixins::KeyLookupAdapter>&& pKeyLookupAdapter)
			: AccountStateCacheDeltaMixins::Size(*accountStateSets.pPrimary)
			, AccountStateCacheDeltaMixins::ContainsAddress(*accountStateSets.pPrimary)
			, AccountStateCacheDeltaMixins::ContainsKey(*accountStateSets.pKeyLookupMap)
			, AccountStateCacheDeltaMixins::ConstAccessorAddress(*accountStateSets.pPrimary)
			, AccountStateCacheDeltaMixins::ConstAccessorKey(*pKeyLookupAdapter)
			, AccountStateCacheDeltaMixins::MutableAccessorAddress(*accountStateSets.pPrimary)
			, AccountStateCacheDeltaMixins::MutableAccessorKey(*pKeyLookupAdapter)
			, AccountStateCacheDeltaMixins::DeltaElements(*accountStateSets.pPrimary)
			, m_pStateByAddress(accountStateSets.pPrimary)
			, m_pKeyToAddress(accountStateSets.pKeyLookupMap)
			, m_options(options)
			, m_highValueAddresses(highValueAddresses)
			, m_pKeyLookupAdapter(std::move(pKeyLookupAdapter))
	{}

	model::NetworkIdentifier BasicAccountStateCacheDelta::networkIdentifier() const {
		return m_options.NetworkIdentifier;
	}

	uint64_t BasicAccountStateCacheDelta::importanceGrouping() const {
		return m_options.ImportanceGrouping;
	}

	Address BasicAccountStateCacheDelta::getAddress(const Key& publicKey) {
		const auto* pPair = m_pKeyToAddress->find(publicKey);
		if (pPair)
			return pPair->second;

		auto address = model::PublicKeyToAddress(publicKey, m_options.NetworkIdentifier);
		m_pKeyToAddress->emplace(publicKey, address);
		return address;
	}

	state::AccountState& BasicAccountStateCacheDelta::addAccount(const Address& address, Height height) {
		auto* pCurrentState = this->tryGet(address);
		if (pCurrentState)
			return *pCurrentState;

		auto pAccountState = std::make_shared<state::AccountState>(address, height);
		m_pStateByAddress->insert(pAccountState);
		return *pAccountState;
	}

	state::AccountState& BasicAccountStateCacheDelta::addAccount(const Key& publicKey, Height height) {
		auto address = getAddress(publicKey);
		auto& accountState = addAccount(address, height);
		if (Height(0) == accountState.PublicKeyHeight) {
			accountState.PublicKey = publicKey;
			accountState.PublicKeyHeight = height;
		}

		return accountState;
	}

	state::AccountState& BasicAccountStateCacheDelta::addAccount(const model::AccountInfo& accountInfo) {
		auto* pCurrentState = this->tryGet(accountInfo.Address);
		if (pCurrentState)
			return *pCurrentState;

		auto pAccountState = std::make_shared<state::AccountState>(state::ToAccountState(accountInfo));
		if (Height(0) != pAccountState->PublicKeyHeight)
			m_pKeyToAddress->emplace(pAccountState->PublicKey, pAccountState->Address);

		m_pStateByAddress->insert(pAccountState);
		return *pAccountState;
	}

	void BasicAccountStateCacheDelta::remove(const Address& address, Height height) {
		const auto* pAccountState = this->tryGet(address);
		if (!pAccountState || height != pAccountState->AddressHeight)
			return;

		// note: we can only remove the entry from m_pKeyToAddress if the account state's public key is valid
		if (Height(0) != pAccountState->PublicKeyHeight)
			m_pKeyToAddress->remove(pAccountState->PublicKey);

		m_pStateByAddress->remove(address);
	}

	void BasicAccountStateCacheDelta::remove(const Key& publicKey, Height height) {
		auto* pAccountState = this->tryGet(publicKey);
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

	void BasicAccountStateCacheDelta::queueRemove(const Address& address, Height height) {
		m_queuedRemoveByAddress.emplace(height, address);
	}

	void BasicAccountStateCacheDelta::queueRemove(const Key& publicKey, Height height) {
		m_queuedRemoveByPublicKey.emplace(height, publicKey);
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
		using DeltasSet = AccountStateCacheTypes::PrimaryTypes::BaseSetDeltaType::SetType::MemorySetType;

		void UpdateAddresses(model::AddressSet& addresses, const DeltasSet& source, const predicate<const state::AccountState&>& include) {
			for (const auto& pair : source) {
				const auto& accountState = *pair.second;
				if (include(accountState))
					addresses.insert(accountState.Address);
				else
					addresses.erase(accountState.Address);
			}
		}
	}

	model::AddressSet BasicAccountStateCacheDelta::highValueAddresses() const {
		// 1. copy original high value addresses
		auto highValueAddresses = m_highValueAddresses;

		// 2. update for changes
		auto hasHighValue = [minBalance = m_options.MinHighValueAccountBalance](const auto& accountState) {
			return accountState.Balances.get(Xem_Id) >= minBalance;
		};

		auto deltas = m_pStateByAddress->deltas();
		UpdateAddresses(highValueAddresses, deltas.Added, hasHighValue);
		UpdateAddresses(highValueAddresses, deltas.Copied, hasHighValue);
		UpdateAddresses(highValueAddresses, deltas.Removed, [](const auto&) { return false; });
		return highValueAddresses;
	}
}}
