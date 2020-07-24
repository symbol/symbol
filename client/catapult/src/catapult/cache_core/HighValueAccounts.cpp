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
#include "catapult/utils/ContainerHelpers.h"

namespace catapult { namespace cache {

	// region HighValueAccounts

	HighValueAccounts::HighValueAccounts()
	{}

	HighValueAccounts::HighValueAccounts(const model::AddressSet& addresses, const AddressAccountHistoryMap& accountHistories)
			: m_addresses(addresses)
			, m_accountHistories(accountHistories)
	{}

	HighValueAccounts::HighValueAccounts(model::AddressSet&& addresses, AddressAccountHistoryMap&& accountHistories)
			: m_addresses(std::move(addresses))
			, m_accountHistories(std::move(accountHistories))
	{}

	const model::AddressSet& HighValueAccounts::addresses() const {
		return m_addresses;
	}

	const AddressAccountHistoryMap& HighValueAccounts::accountHistories() const {
		return m_accountHistories;
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
				for (const auto& pair : source)
					updateOne(pair.second.Address, include(pair.second));
			}

		private:
			void updateOne(const Address& address, bool shouldInclude) {
				if (shouldInclude) {
					m_current.insert(address);

					// needed for multiblock syncs when original account is removed and then readded
					m_removed.erase(address);
				} else {
					m_current.erase(address);

					if (m_original.cend() != m_original.find(address))
						m_removed.insert(address);
				}
			}

		private:
			const model::AddressSet& m_original;
			model::AddressSet& m_current;
			model::AddressSet& m_removed;
		};
	}

	// endregion

	// region HighValueBalancesUpdater

	namespace {
		using EffectiveBalanceCalculator = std::function<std::pair<Amount, bool> (const state::AccountState&)>;

		class HighValueBalancesUpdater {
		private:
			using MemorySetType = AccountStateCacheTypes::PrimaryTypes::BaseSetDeltaType::SetType::MemorySetType;

		public:
			HighValueBalancesUpdater(AddressAccountHistoryMap& accountHistories, Height height)
					: m_accountHistories(accountHistories)
					, m_height(height)
			{}

		public:
			void update(const MemorySetType& source, const EffectiveBalanceCalculator& effectiveBalanceCalculator) {
				for (const auto& pair : source)
					updateOne(pair.second, effectiveBalanceCalculator(pair.second));
			}

			void prune(Amount minBalance) {
				utils::map_erase_if(m_accountHistories, [minBalance](const auto& pair) {
					return !pair.second.anyAtLeast(minBalance);
				});
			}

		private:
			void updateOne(const state::AccountState& accountState, const std::pair<Amount, bool>& effectiveBalancePair) {
				auto accountHistoriesIter = m_accountHistories.find(accountState.Address);

				// if this account has a newly high balance, start tracking it
				if (m_accountHistories.cend() == accountHistoriesIter && effectiveBalancePair.second)
					accountHistoriesIter = m_accountHistories.emplace(accountState.Address, state::AccountHistory()).first;

				// if this account is tracked, add tracked values
				if (m_accountHistories.cend() != accountHistoriesIter) {
					accountHistoriesIter->second.add(m_height, effectiveBalancePair.first);
					accountHistoriesIter->second.add(m_height, accountState.SupplementalPublicKeys.vrf().get());
					accountHistoriesIter->second.add(m_height, accountState.SupplementalPublicKeys.voting().getAll());
				}
			}

		private:
			AddressAccountHistoryMap& m_accountHistories;
			Height m_height;
		};
	}

	// endregion

	// region HighValueAccountsUpdater

	HighValueAccountsUpdater::HighValueAccountsUpdater(const AccountStateCacheTypes::Options& options, const HighValueAccounts& accounts)
			: m_options(options)
			, m_original(accounts.addresses())
			, m_current(accounts.addresses())
			, m_accountHistories(accounts.accountHistories())
			, m_height(Height(1))
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

	const AddressAccountHistoryMap& HighValueAccountsUpdater::accountHistories() const {
		return m_accountHistories;
	}

	void HighValueAccountsUpdater::setHeight(Height height) {
		m_height = height;
	}

	void HighValueAccountsUpdater::update(const deltaset::DeltaElements<MemorySetType>& deltas) {
		updateHarvestingAccounts(deltas);
		updateVotingAccounts(deltas);
	}

	void HighValueAccountsUpdater::prune(Height height) {
		utils::map_erase_if(m_accountHistories, [height, minBalance = m_options.MinVoterBalance](auto& pair) {
			pair.second.prune(height);
			return !pair.second.anyAtLeast(minBalance);
		});
	}

	HighValueAccounts HighValueAccountsUpdater::detachAccounts() {
		auto accounts = HighValueAccounts(std::move(m_current), std::move(m_accountHistories));

		m_current.clear();
		m_removed.clear();
		m_accountHistories.clear();

		return accounts;
	}

	namespace {
		EffectiveBalanceCalculator::result_type EffectiveBalanceRetriever(
				const state::AccountState& accountState,
				MosaicId harvestingMosaicId,
				Amount minBalance) {
			auto balance = accountState.Balances.get(harvestingMosaicId);
			return std::make_pair(balance, balance >= minBalance);
		}
	}

	void HighValueAccountsUpdater::updateHarvestingAccounts(const deltaset::DeltaElements<MemorySetType>& deltas) {
		auto hasHighValue = [&options = m_options](const auto& accountState) {
			return EffectiveBalanceRetriever(accountState, options.HarvestingMosaicId, options.MinHarvesterBalance).second;
		};

		HighValueAddressesUpdater updater(m_original, m_current, m_removed);
		updater.update(deltas.Added, hasHighValue);
		updater.update(deltas.Copied, hasHighValue);
		updater.update(deltas.Removed, [](const auto&) { return false; });
	}

	void HighValueAccountsUpdater::updateVotingAccounts(const deltaset::DeltaElements<MemorySetType>& deltas) {
		auto effectiveBalanceCalculator = [&options = m_options](const auto& accountState) {
			if (0 != accountState.SupplementalPublicKeys.voting().size()) {
				auto balancePair = EffectiveBalanceRetriever(accountState, options.HarvestingMosaicId, options.MinVoterBalance);
				if (balancePair.second)
					return balancePair;
			}

			return std::make_pair(Amount(), false);
		};

		HighValueBalancesUpdater updater(m_accountHistories, m_height);
		updater.update(deltas.Added, effectiveBalanceCalculator);
		updater.update(deltas.Copied, effectiveBalanceCalculator);
		updater.update(deltas.Removed, [](const auto&) { return std::make_pair(Amount(), false); });
		updater.prune(m_options.MinVoterBalance);
	}

	// endregion
}}
