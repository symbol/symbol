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

#include "HighValueAccounts.h"
#include "catapult/utils/ContainerHelpers.h"

namespace catapult { namespace cache {

	// region HighValueAccounts

	HighValueAccounts::HighValueAccounts()
	{}

	HighValueAccounts::HighValueAccounts(
			const model::AddressSet& addresses,
			const model::AddressSet& removedAddresses,
			const AddressAccountHistoryMap& accountHistories)
			: m_addresses(addresses)
			, m_removedAddresses(removedAddresses)
			, m_accountHistories(accountHistories)
	{}

	HighValueAccounts::HighValueAccounts(
			model::AddressSet&& addresses,
			model::AddressSet&& removedAddresses,
			AddressAccountHistoryMap&& accountHistories)
			: m_addresses(std::move(addresses))
			, m_removedAddresses(removedAddresses)
			, m_accountHistories(std::move(accountHistories))
	{}

	const model::AddressSet& HighValueAccounts::addresses() const {
		return m_addresses;
	}

	const model::AddressSet& HighValueAccounts::removedAddresses() const {
		return m_removedAddresses;
	}

	const AddressAccountHistoryMap& HighValueAccounts::accountHistories() const {
		return m_accountHistories;
	}

	// endregion

	// region HighValueAddressesUpdater

	namespace {
		struct HighValueAccountDescriptor {
			bool IsHighValue;
			bool HasHistoricalInformation;
		};

		using HighValueAccountDescriptorCalculator = std::function<HighValueAccountDescriptor (const state::AccountState&)>;

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
			void update(const MemorySetType& source, const HighValueAccountDescriptorCalculator& highValueAccountDescriptorCalculator) {
				for (const auto& pair : source)
					updateOne(pair.second.Address, highValueAccountDescriptorCalculator(pair.second));
			}

		private:
			void updateOne(const Address& address, const HighValueAccountDescriptor& descriptor) {
				if (descriptor.IsHighValue) {
					m_current.insert(address);
					m_removed.erase(address);
				} else {
					m_current.erase(address);

					// need to check HasHistoricalInformation in order for multiblock syncs to work
					if (m_original.cend() != m_original.find(address) || descriptor.HasHistoricalInformation)
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

			void pruneGreater() {
				for (auto& pair : m_accountHistories)
					pair.second.pruneGreater(m_height);
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
			, m_removed(accounts.removedAddresses())
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

	void HighValueAccountsUpdater::setRemovedAddresses(model::AddressSet&& removedAddresses) {
		m_removed = std::move(removedAddresses);
	}

	void HighValueAccountsUpdater::update(const deltaset::DeltaElements<MemorySetType>& deltas) {
		updateHarvestingAccounts(deltas);
		updateVotingAccounts(deltas);
	}

	void HighValueAccountsUpdater::prune(Height height) {
		utils::map_erase_if(m_accountHistories, [height, minBalance = m_options.MinVoterBalance](auto& pair) {
			pair.second.pruneLess(height);
			return !pair.second.anyAtLeast(minBalance);
		});
	}

	HighValueAccounts HighValueAccountsUpdater::detachAccounts() {
		auto accounts = HighValueAccounts(std::move(m_current), std::move(m_removed), std::move(m_accountHistories));

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
			return HighValueAccountDescriptor{
				EffectiveBalanceRetriever(accountState, options.HarvestingMosaicId, options.MinHarvesterBalance).second,
				state::HasHistoricalInformation(accountState)
			};
		};

		HighValueAddressesUpdater updater(m_original, m_current, m_removed);
		updater.update(deltas.Added, hasHighValue);
		updater.update(deltas.Copied, hasHighValue);
		updater.update(deltas.Removed, [](const auto&) { return HighValueAccountDescriptor{ false, false }; });
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
		updater.pruneGreater();
		updater.update(deltas.Added, effectiveBalanceCalculator);
		updater.update(deltas.Copied, effectiveBalanceCalculator);
		updater.update(deltas.Removed, [](const auto&) { return std::make_pair(Amount(), false); });
		updater.prune(m_options.MinVoterBalance);
	}

	// endregion
}}
