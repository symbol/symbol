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

#include "HighValueAccounts.h"

namespace catapult { namespace cache {

	// region HighValueAccounts

	HighValueAccounts::HighValueAccounts()
	{}

	HighValueAccounts::HighValueAccounts(const model::AddressSet& addresses) : m_addresses(addresses)
	{}

	HighValueAccounts::HighValueAccounts(model::AddressSet&& addresses) : m_addresses(std::move(addresses))
	{}

	const model::AddressSet& HighValueAccounts::addresses() const {
		return m_addresses;
	}

	// endregion

	// region HighValueAddressesUpdater

	namespace {
		class HighValueAddressesUpdater {
		private:
			using MemorySetType = AccountStateCacheTypes::PrimaryTypes::BaseSetDeltaType::SetType::MemorySetType;

		public:
			HighValueAddressesUpdater(
					const model::AddressSet& originalAddresses,
					model::AddressSet& currentAddresses,
					model::AddressSet& removedAddresses)
					: m_original(originalAddresses)
					, m_current(currentAddresses)
					, m_removed(removedAddresses)
			{}

		public:
			void update(const MemorySetType& source, const predicate<const state::AccountState&>& include) {
				for (const auto& pair : source) {
					const auto& accountState = pair.second;
					const auto& address = accountState.Address;
					if (include(accountState)) {
						m_current.insert(address);

						// don't need to modify m_removed because an element can't be in both Added and Copied
					} else {
						m_current.erase(address);

						if (m_original.cend() != m_original.find(address))
							m_removed.insert(address);
					}
				}
			}

		private:
			const model::AddressSet& m_original;
			model::AddressSet& m_current;
			model::AddressSet& m_removed;
		};
	}

	// endregion

	// region HighValueAccountsUpdater

	HighValueAccountsUpdater::HighValueAccountsUpdater(AccountStateCacheTypes::Options options, const model::AddressSet& addresses)
			: m_options(options)
			, m_original(addresses)
			, m_current(addresses)
	{}

	Height HighValueAccountsUpdater::height() const {
		return m_height;
	}

	const model::AddressSet& HighValueAccountsUpdater::addresses() const {
		return m_current;
	}

	const model::AddressSet& HighValueAccountsUpdater::removedAddresses() const {
		return m_removed;
	}

	void HighValueAccountsUpdater::setHeight(Height height) {
		m_height = height;
	}

	void HighValueAccountsUpdater::update(const deltaset::DeltaElements<MemorySetType>& deltas) {
		auto minBalance = m_options.MinHarvesterBalance;
		auto harvestingMosaicId = m_options.HarvestingMosaicId;
		auto hasHighValue = [minBalance, harvestingMosaicId](const auto& accountState) {
			return accountState.Balances.get(harvestingMosaicId) >= minBalance;
		};

		HighValueAddressesUpdater updater(m_original, m_current, m_removed);
		updater.update(deltas.Added, hasHighValue);
		updater.update(deltas.Copied, hasHighValue);
		updater.update(deltas.Removed, [](const auto&) { return false; });
	}

	HighValueAccounts HighValueAccountsUpdater::detachAccounts() {
		auto accounts = HighValueAccounts(std::move(m_current));

		m_current.clear();
		m_removed.clear();

		return accounts;
	}

	// endregion
}}
