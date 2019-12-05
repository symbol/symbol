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
#include "AccountStateBaseSets.h"
#include "AccountStateCacheSerializers.h"
#include "ReadOnlyAccountStateCache.h"
#include "catapult/cache/CacheMixinAliases.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "catapult/model/ContainerTypes.h"

namespace catapult { namespace cache {

	/// Mixins used by the account state cache delta.
	struct AccountStateCacheDeltaMixins {
	public:
		using KeyLookupAdapter = AccountStateCacheTypes::ComposedLookupAdapter<AccountStateCacheTypes::ComposableBaseSetDeltas>;

	private:
		using AddressMixins = PatriciaTreeCacheMixins<AccountStateCacheTypes::PrimaryTypes::BaseSetDeltaType, AccountStateCacheDescriptor>;
		using KeyMixins = BasicCacheMixins<KeyLookupAdapter, KeyLookupAdapter>;

	public:
		using Size = AddressMixins::Size;
		using ContainsAddress = AddressMixins::Contains;
		using ContainsKey = ContainsMixin<
			AccountStateCacheTypes::KeyLookupMapTypes::BaseSetDeltaType,
			AccountStateCacheTypes::KeyLookupMapTypesDescriptor>;
		using ConstAccessorAddress = AddressMixins::ConstAccessor;
		using ConstAccessorKey = KeyMixins::ConstAccessor;
		using MutableAccessorAddress = AddressMixins::MutableAccessor;
		using MutableAccessorKey = KeyMixins::MutableAccessor;
		using PatriciaTreeDelta = AddressMixins::PatriciaTreeDelta;
		using DeltaElements = AddressMixins::DeltaElements;

		// no mutable key accessor because address-to-key pairs are immutable
	};

	/// Basic delta on top of the account state cache.
	class BasicAccountStateCacheDelta
			: public utils::MoveOnly
			, public AccountStateCacheDeltaMixins::Size
			, public AccountStateCacheDeltaMixins::ContainsAddress
			, public AccountStateCacheDeltaMixins::ContainsKey
			, public AccountStateCacheDeltaMixins::ConstAccessorAddress
			, public AccountStateCacheDeltaMixins::ConstAccessorKey
			, public AccountStateCacheDeltaMixins::MutableAccessorAddress
			, public AccountStateCacheDeltaMixins::MutableAccessorKey
			, public AccountStateCacheDeltaMixins::PatriciaTreeDelta
			, public AccountStateCacheDeltaMixins::DeltaElements {
	public:
		using ReadOnlyView = ReadOnlyAccountStateCache;

	public:
		/// Creates a delta around \a accountStateSets, \a options and \a highValueAddresses.
		BasicAccountStateCacheDelta(
				const AccountStateCacheTypes::BaseSetDeltaPointers& accountStateSets,
				const AccountStateCacheTypes::Options& options,
				const model::AddressSet& highValueAddresses);

	private:
		BasicAccountStateCacheDelta(
				const AccountStateCacheTypes::BaseSetDeltaPointers& accountStateSets,
				const AccountStateCacheTypes::Options& options,
				const model::AddressSet& highValueAddresses,
				std::unique_ptr<AccountStateCacheDeltaMixins::KeyLookupAdapter>&& pKeyLookupAdapter);

	public:
		using AccountStateCacheDeltaMixins::ContainsAddress::contains;
		using AccountStateCacheDeltaMixins::ContainsKey::contains;

		using AccountStateCacheDeltaMixins::ConstAccessorAddress::find;
		using AccountStateCacheDeltaMixins::ConstAccessorKey::find;
		using AccountStateCacheDeltaMixins::MutableAccessorAddress::find;
		using AccountStateCacheDeltaMixins::MutableAccessorKey::find;

	public:
		/// Gets the network identifier.
		model::NetworkIdentifier networkIdentifier() const;

		/// Gets the network importance grouping.
		uint64_t importanceGrouping() const;

		/// Gets the minimum harvester balance.
		Amount minHarvesterBalance() const;

		/// Gets the maximum harvester balance.
		Amount maxHarvesterBalance() const;

		/// Gets the harvesting mosaic id.
		MosaicId harvestingMosaicId() const;

	public:
		/// If not present, adds an account to the cache at given height (\a addressHeight) using \a address.
		void addAccount(const Address& address, Height addressHeight);

		/// If not present, adds an account to the cache using \a publicKey.
		/// If public key has not been known earlier, its height is set to \a publicKeyHeight.
		void addAccount(const Key& publicKey, Height publicKeyHeight);

		/// If not present, adds an account to the cache using information in \a accountState.
		void addAccount(const state::AccountState& accountState);

	public:
		/// If \a height matches the height at which account was added, queues removal of account's \a address
		/// information from the cache, therefore queuing complete removal of the account from the cache.
		void queueRemove(const Address& address, Height height);

		/// If \a height matches the height at which account was added, queues removal of account's \a publicKey
		/// information from the cache.
		void queueRemove(const Key& publicKey, Height height);

		/// Clears any queued removals for \a address at \a height
		void clearRemove(const Address& address, Height height);

		/// Clears any queued removals for \a publicKey at \a height.
		void clearRemove(const Key& publicKey, Height height);

		/// Commits all queued removals.
		void commitRemovals();

	public:
		/// Tuple composed of information about high value addresses that is returned by highValueAddresses.
		struct HighValueAddressesTuple {
			/// Addresses of accounts that are high value after application of all delta changes.
			model::AddressSet Current;

			/// Addresses of accounts that were high value but are no longer high value after application of all delta changes.
			model::AddressSet Removed;
		};

		/// Gets all high value addresses.
		HighValueAddressesTuple highValueAddresses() const;

	private:
		Address getAddress(const Key& publicKey);

		void remove(const Address& address, Height height);
		void remove(const Key& publicKey, Height height);

	private:
		// height is first component for a nicer equals
		template<typename TArray>
		using ArrayHeightPair = std::pair<Height, TArray>;

		template<typename TArray>
		struct ArrayHeightPairHasher {
			size_t operator()(const ArrayHeightPair<TArray>& arrayHeightPair) const {
				return Hasher(arrayHeightPair.second);
			}

			utils::ArrayHasher<TArray> Hasher;
		};

		template<typename TArray>
		using QueuedRemovalSet = std::unordered_set<ArrayHeightPair<TArray>, ArrayHeightPairHasher<TArray>>;

	private:
		AccountStateCacheTypes::PrimaryTypes::BaseSetDeltaPointerType m_pStateByAddress;
		AccountStateCacheTypes::KeyLookupMapTypes::BaseSetDeltaPointerType m_pKeyToAddress;

		const AccountStateCacheTypes::Options& m_options;
		const model::AddressSet& m_highValueAddresses;
		std::unique_ptr<AccountStateCacheDeltaMixins::KeyLookupAdapter> m_pKeyLookupAdapter;

		QueuedRemovalSet<Address> m_queuedRemoveByAddress;
		QueuedRemovalSet<Key> m_queuedRemoveByPublicKey;
	};

	/// Delta on top of the account state cache.
	class AccountStateCacheDelta : public ReadOnlyViewSupplier<BasicAccountStateCacheDelta> {
	public:
		/// Creates a delta around \a accountStateSets, \a options and \a highValueAddresses.
		AccountStateCacheDelta(
				const AccountStateCacheTypes::BaseSetDeltaPointers& accountStateSets,
				const AccountStateCacheTypes::Options& options,
				const model::AddressSet& highValueAddresses)
				: ReadOnlyViewSupplier(accountStateSets, options, highValueAddresses)
		{}
	};
}}
