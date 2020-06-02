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

#pragma once
#include "AccountStateCacheTypes.h"
#include "catapult/model/ContainerTypes.h"
#include "catapult/state/BalanceHistory.h"

namespace catapult { namespace cache {

	/// Map of addresses to balance histories.
	using AddressBalanceHistoryMap = std::unordered_map<Address, state::BalanceHistory, utils::ArrayHasher<Address>>;

	/// High value accounts container.
	class HighValueAccounts {
	public:
		/// Creates an empty container.
		HighValueAccounts();

		/// Creates a container around \a addresses and \a balanceHistories.
		HighValueAccounts(const model::AddressSet& addresses, const AddressBalanceHistoryMap& balanceHistories);

		/// Creates a container around \a addresses and \a balanceHistories.
		HighValueAccounts(model::AddressSet&& addresses, AddressBalanceHistoryMap&& balanceHistories);

	public:
		/// Gets the high value (harvester eligible) addresses.
		const model::AddressSet& addresses() const;

		/// Gets the high value (voter eligible) balance histories.
		const AddressBalanceHistoryMap& balanceHistories() const;

	private:
		model::AddressSet m_addresses;
		AddressBalanceHistoryMap m_balanceHistories;
	};

	/// High value accounts updater.
	class HighValueAccountsUpdater {
	private:
		using MemorySetType = AccountStateCacheTypes::PrimaryTypes::BaseSetDeltaType::SetType::MemorySetType;

	public:
		/// Creates an updater around \a options and existing \a accounts.
		HighValueAccountsUpdater(const AccountStateCacheTypes::Options& options, const HighValueAccounts& accounts);

	public:
		/// Gets the height of the update operation.
		Height height() const;

		/// Gets the (current) high value (harvester eligible) addresses.
		const model::AddressSet& addresses() const;

		/// Gets the (removed) high value (harvester eligible) addresses relative to the initial addresses.
		const model::AddressSet& removedAddresses() const;

		/// Gets the high value (voter eligible) balance histories.
		const AddressBalanceHistoryMap& balanceHistories() const;

	public:
		/// Sets the \a height of the update operation.
		void setHeight(Height height);

		/// Updates high value accounts based on changes described in \a deltas.
		void update(const deltaset::DeltaElements<MemorySetType>& deltas);

		/// Prunes all balances less than \a height.
		void prune(Height height);

	public:
		/// Detaches the underlying data associated with this updater and converts it to a high value accounts container.
		HighValueAccounts detachAccounts();

	private:
		void updateHarvestingAccounts(const deltaset::DeltaElements<MemorySetType>& deltas);
		void updateVotingAccounts(const deltaset::DeltaElements<MemorySetType>& deltas);

	private:
		AccountStateCacheTypes::Options m_options;
		const model::AddressSet& m_original;
		model::AddressSet m_current;
		model::AddressSet m_removed;
		AddressBalanceHistoryMap m_balanceHistories;
		Height m_height;
	};
}}
