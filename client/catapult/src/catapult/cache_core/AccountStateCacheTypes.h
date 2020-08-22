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
#include "catapult/cache/CacheDatabaseMixin.h"
#include "catapult/cache/CacheDescriptorAdapters.h"
#include "catapult/deltaset/BaseSetDelta.h"
#include "catapult/model/NetworkIdentifier.h"
#include "catapult/state/AccountState.h"
#include "catapult/utils/Casting.h"
#include "catapult/utils/Hashers.h"

namespace catapult {
	namespace cache {
		struct AccountStateBaseSetDeltaPointers;
		struct AccountStateBaseSets;
		class AccountStateCache;
		class AccountStateCacheDelta;
		class AccountStateCacheView;
		class AccountStatePatriciaTree;
		struct AccountStatePrimarySerializer;
		class BasicAccountStateCacheDelta;
		class BasicAccountStateCacheView;
		struct KeyAddressPairSerializer;
		class ReadOnlyAccountStateCache;
	}
}

namespace catapult { namespace cache {

	/// Describes an account state cache.
	struct AccountStateCacheDescriptor {
	public:
		static constexpr auto Name = "AccountStateCache:Address";

	public:
		// key value types
		using KeyType = Address;
		using ValueType = state::AccountState;

		// cache types
		using CacheType = AccountStateCache;
		using CacheDeltaType = AccountStateCacheDelta;
		using CacheViewType = AccountStateCacheView;

		using Serializer = AccountStatePrimarySerializer;
		using PatriciaTree = AccountStatePatriciaTree;

	public:
		/// Gets the key corresponding to \a accountState.
		static auto GetKeyFromValue(const ValueType& accountState) {
			return accountState.Address;
		}
	};

	/// AccountState cache types.
	struct AccountStateCacheTypes {
	public:
		using CacheReadOnlyType = ReadOnlyAccountStateCache;

		/// Custom sub view options.
		struct Options {
			/// Network identifier.
			model::NetworkIdentifier NetworkIdentifier;

			/// Importance grouping.
			uint64_t ImportanceGrouping;

			/// Voting set grouping.
			uint64_t VotingSetGrouping;

			/// Minimum harvester balance.
			/// \note This doubles as the minimum balance of tracked high value accounts.
			Amount MinHarvesterBalance;

			/// Maximum harvester balance.
			Amount MaxHarvesterBalance;

			/// Minimum voter balance.
			/// \note This doubles as the minimum balance of tracked high value accounts with balances.
			Amount MinVoterBalance;

			/// Mosaic id used as primary chain currency.
			MosaicId CurrencyMosaicId;

			/// Mosaic id used to provide harvesting ability.
			MosaicId HarvestingMosaicId;
		};

	// region secondary descriptors

	public:
		/// Describes a key-based interface on top of an account state cache.
		struct KeyLookupMapTypesDescriptor {
		public:
			using KeyType = Key;
			using ValueType = std::pair<Key, Address>;
			using Serializer = KeyAddressPairSerializer;

		public:
			static auto GetKeyFromValue(const ValueType& pair) {
				return pair.first;
			}
		};

	// endregion

	// region other adapters

	public:
		/// Adapter for a dual lookup.
		/// \note This allows use of mixins.
		template<typename TSets>
		class ComposedLookupAdapter {
		public:
			static constexpr auto Name = TSets::Name;

		private:
			using SetOneType = typename TSets::SetOneType;
			using SetTwoType = typename TSets::SetTwoType;

		public:
			using KeyType = typename SetOneType::KeyType;
			using ValueType = typename SetTwoType::ElementType;

		public:
			ComposedLookupAdapter(const SetOneType& set1, SetTwoType& set2)
					: m_set1(set1)
					, m_set2(set2)
			{}

		public:
			using FindIterator = typename TSets::FindIterator;
			using FindConstIterator = typename TSets::FindConstIterator;

		public:
			FindConstIterator find(const KeyType& key) const {
				auto setOneIter = m_set1.find(key);
				const auto* pPair = setOneIter.get();
				return pPair ? utils::as_const(m_set2).find(pPair->second) : FindConstIterator();
			}

			FindIterator find(const KeyType& key) {
				auto setOneIter = m_set1.find(key);
				const auto* pPair = setOneIter.get();
				return pPair ? m_set2.find(pPair->second) : FindIterator();
			}

		private:
			const SetOneType& m_set1;
			SetTwoType& m_set2;
		};

	// endregion

	public:
		using PrimaryTypes = MutableUnorderedMapAdapter<AccountStateCacheDescriptor, utils::ArrayHasher<Address>>;
		using KeyLookupMapTypes = ImmutableUnorderedMapAdapter<KeyLookupMapTypesDescriptor, utils::ArrayHasher<Key>>;

	public:
		// workaround for VS truncation
		struct ComposableBaseSets {
			static constexpr auto Name = "AccountStateCache:Key";

			using SetOneType = const KeyLookupMapTypes::BaseSetType;
			using SetTwoType = const PrimaryTypes::BaseSetType;

			using FindIterator = SetTwoType::FindConstIterator;
			using FindConstIterator = SetTwoType::FindConstIterator;
		};

		struct ComposableBaseSetDeltas {
			static constexpr auto Name = "AccountStateCache:Key";

			using SetOneType = const KeyLookupMapTypes::BaseSetDeltaType;
			using SetTwoType = PrimaryTypes::BaseSetDeltaType;

			using FindIterator = SetTwoType::FindIterator;
			using FindConstIterator = SetTwoType::FindConstIterator;
		};

	public:
		using BaseSetDeltaPointers = AccountStateBaseSetDeltaPointers;
		using BaseSets = AccountStateBaseSets;
	};
}}
