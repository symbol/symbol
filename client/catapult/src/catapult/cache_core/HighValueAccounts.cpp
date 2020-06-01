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

	HighValueAccounts::HighValueAccounts(const model::AddressSet& addresses, const AddressBalanceHistoryMap& balanceHistories)
			: m_addresses(addresses)
			, m_balanceHistories(balanceHistories)
	{}

	HighValueAccounts::HighValueAccounts(model::AddressSet&& addresses, AddressBalanceHistoryMap&& balanceHistories)
			: m_addresses(std::move(addresses))
			, m_balanceHistories(std::move(balanceHistories))
	{}

	const model::AddressSet& HighValueAccounts::addresses() const {
		return m_addresses;
	}

	const AddressBalanceHistoryMap& HighValueAccounts::balanceHistories() const {
		return m_balanceHistories;
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

					// don't need to modify m_removed because an element can't be in both Added and Copied
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
			HighValueBalancesUpdater(AddressBalanceHistoryMap& balanceHistories, Height height)
					: m_balanceHistories(balanceHistories)
					, m_height(height)
			{}

		public:
			void update(const MemorySetType& source, const EffectiveBalanceCalculator& effectiveBalanceCalculator) {
				for (const auto& pair : source)
					updateOne(pair.second.Address, effectiveBalanceCalculator(pair.second));
			}

			void prune(Amount minBalance) {
				utils::map_erase_if(m_balanceHistories, [minBalance](const auto& pair) {
					return !pair.second.anyAtLeast(minBalance);
				});
			}

		private:
			void updateOne(const Address& address, const std::pair<Amount, bool>& effectiveBalancePair) {
				auto balanceHistoriesIter = m_balanceHistories.find(address);

				// if this address has a newly high balance, start tracking it
				if (m_balanceHistories.cend() == balanceHistoriesIter && effectiveBalancePair.second)
					balanceHistoriesIter = m_balanceHistories.emplace(address, state::BalanceHistory()).first;

				// if this address is tracked, add balance
				if (m_balanceHistories.cend() != balanceHistoriesIter)
					balanceHistoriesIter->second.add(m_height, effectiveBalancePair.first);
			}

		private:
			AddressBalanceHistoryMap& m_balanceHistories;
			Height m_height;
		};
	}

	// endregion

	// region HighValueAccountsUpdater

	HighValueAccountsUpdater::HighValueAccountsUpdater(const AccountStateCacheTypes::Options& options, const HighValueAccounts& accounts)
			: m_options(options)
			, m_original(accounts.addresses())
			, m_current(accounts.addresses())
			, m_balanceHistories(accounts.balanceHistories())
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

	const AddressBalanceHistoryMap& HighValueAccountsUpdater::balanceHistories() const {
		return m_balanceHistories;
	}

	void HighValueAccountsUpdater::setHeight(Height height) {
		m_height = height;
	}

	void HighValueAccountsUpdater::update(const deltaset::DeltaElements<MemorySetType>& deltas) {
		updateHarvestingAccounts(deltas);
		updateVotingAccounts(deltas);
	}

	void HighValueAccountsUpdater::prune(Height height) {
		utils::map_erase_if(m_balanceHistories, [height, minBalance = m_options.MinVoterBalance](auto& pair) {
			pair.second.prune(height);
			return !pair.second.anyAtLeast(minBalance);
		});
	}

	HighValueAccounts HighValueAccountsUpdater::detachAccounts() {
		auto accounts = HighValueAccounts(std::move(m_current), std::move(m_balanceHistories));

		m_current.clear();
		m_removed.clear();
		m_balanceHistories.clear();

		return accounts;
	}

	namespace {
		EffectiveBalanceCalculator CreateEffectiveBalanceCalculator(MosaicId harvestingMosaicId, Amount minBalance) {
			return [harvestingMosaicId, minBalance](const auto& accountState) {
				auto balance = accountState.Balances.get(harvestingMosaicId);
				return std::make_pair(balance, balance >= minBalance);
			};
		}

		predicate<const state::AccountState&> CreateHasHighValuePredicate(MosaicId harvestingMosaicId, Amount minBalance) {
			auto effectiveBalanceCalculator = CreateEffectiveBalanceCalculator(harvestingMosaicId, minBalance);
			return [effectiveBalanceCalculator](const auto& accountState) {
				return effectiveBalanceCalculator(accountState).second;
			};
		}
	}

	void HighValueAccountsUpdater::updateHarvestingAccounts(const deltaset::DeltaElements<MemorySetType>& deltas) {
		auto hasHighValue = CreateHasHighValuePredicate(m_options.HarvestingMosaicId, m_options.MinHarvesterBalance);

		HighValueAddressesUpdater updater(m_original, m_current, m_removed);
		updater.update(deltas.Added, hasHighValue);
		updater.update(deltas.Copied, hasHighValue);
		updater.update(deltas.Removed, [](const auto&) { return false; });
	}

	void HighValueAccountsUpdater::updateVotingAccounts(const deltaset::DeltaElements<MemorySetType>& deltas) {
		auto effectiveBalanceCalculator = CreateEffectiveBalanceCalculator(m_options.HarvestingMosaicId, m_options.MinVoterBalance);

		HighValueBalancesUpdater updater(m_balanceHistories, m_height);
		updater.update(deltas.Added, effectiveBalanceCalculator);
		updater.update(deltas.Copied, effectiveBalanceCalculator);
		updater.update(deltas.Removed, [](const auto&) { return std::make_pair(Amount(), false); });
		updater.prune(m_options.MinVoterBalance);
	}

	// endregion
}}
