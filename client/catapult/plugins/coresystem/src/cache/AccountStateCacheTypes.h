#pragma once
#include "catapult/deltaset/BaseSet.h"
#include "catapult/state/AccountState.h"
#include "catapult/utils/Hashers.h"
#include <memory>
#include <unordered_set>

namespace catapult { namespace cache { class ReadOnlyAccountStateCache; } }

namespace catapult { namespace cache {

	namespace account_state_cache_types {
		/// The cache pointer type.
		using PointerType = std::shared_ptr<state::AccountState>;

		/// The cache const pointer type.
		using ConstPointerType = std::shared_ptr<const state::AccountState>;

		/// A read-only view of an account state cache.
		using CacheReadOnlyType = ReadOnlyAccountStateCache;

		namespace address_account_state_map {
			/// The account state entity traits.
			using EntityTraits = deltaset::MutableTypeTraits<PointerType>;

			/// The account state underlying map.
			using AddressBasedMap = std::unordered_map<
				Address,
				PointerType,
				utils::ArrayHasher<Address>>;

			/// Retrieves the map key from \a pState.
			struct AccountStateToAddressConverter {
				static auto ToKey(const PointerType& pState) {
					return pState->Address;
				}
			};

			/// The base set type.
			using BaseSetType = deltaset::BaseSet<
				EntityTraits,
				deltaset::MapStorageTraits<AddressBasedMap, AccountStateToAddressConverter>>;

			/// The base set delta type.
			using BaseSetDeltaType = BaseSetType::DeltaType;

			/// A pointer to the base set delta type.
			using BaseSetDeltaPointerType = std::shared_ptr<BaseSetDeltaType>;
		}

		namespace key_address_map {
			/// A key address pair.
			using KeyAddressPair = std::pair<Key, Address>;

			/// Retrieves the map key from \a pair
			struct KeyAddressPairConverter {
				static auto ToKey(const KeyAddressPair& pair) {
					return pair.first;
				}
			};

			/// The key address pair entity traits.
			using EntityTraits = deltaset::ImmutableTypeTraits<KeyAddressPair>;

			/// The key address pair underlying map.
			using KeyAddressPairMap = std::unordered_map<
				Key,
				KeyAddressPair,
				utils::ArrayHasher<Key>>;

			/// The base set type.
			using BaseSetType = deltaset::BaseSet<
				EntityTraits,
				deltaset::MapStorageTraits<KeyAddressPairMap, KeyAddressPairConverter>>;

			/// The base set delta type.
			using BaseSetDeltaType = BaseSetType::DeltaType;

			/// A pointer to the base set delta type.
			using BaseSetDeltaPointerType = std::shared_ptr<BaseSetDeltaType>;
		}

		/// A (key/address) array and height pair.
		/// \note Pass Height as first for a bit nicer operator==.
		template<typename TArray>
		using ArrayHeightPair = std::pair<Height, TArray>;

		/// A hasher for ArrayHeightPair.
		template<typename TArray>
		struct ArrayHeightPairHasher {
			size_t operator()(const ArrayHeightPair<TArray>& arrayHeightPair) const {
				return Hasher(arrayHeightPair.second);
			}

			utils::ArrayHasher<TArray> Hasher;
		};

		/// A set containing queued removals.
		template<typename TArray>
		using QueuedRemovalSet = std::unordered_set<ArrayHeightPair<TArray>, ArrayHeightPairHasher<TArray>>;
	}
}}
