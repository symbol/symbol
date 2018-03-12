#pragma once
#include "catapult/cache/CacheDescriptorAdapters.h"
#include "catapult/deltaset/BaseSetDelta.h"
#include "catapult/model/NetworkInfo.h"
#include "catapult/state/AccountState.h"
#include "catapult/utils/Casting.h"
#include "catapult/utils/Hashers.h"

namespace catapult {
	namespace cache {
		class AccountStateCache;
		class AccountStateCacheDelta;
		class AccountStateCacheView;
		class BasicAccountStateCacheDelta;
		class BasicAccountStateCacheView;
		class ReadOnlyAccountStateCache;
	}
}

namespace catapult { namespace cache {

	/// Describes an account state cache.
	struct AccountStateCacheDescriptor {
	public:
		// key value types
		using KeyType = Address;
		using ValueType = std::shared_ptr<state::AccountState>;

		// cache types
		using CacheType = AccountStateCache;
		using CacheDeltaType = AccountStateCacheDelta;
		using CacheViewType = AccountStateCacheView;

	public:
		/// Gets the key corresponding to \a pAccountState.
		static auto GetKeyFromValue(const ValueType& pAccountState) {
			return pAccountState->Address;
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

			/// Minimum high value account balance.
			Amount MinHighValueAccountBalance;
		};

	// region value adapters

	private:
		// notice that ValueAdapter is still needed to map ValueType (shared_ptr<T>) to TAccountState via AdaptedValueType
		template<typename TAccountState>
		struct ValueAdapter {
			using AdaptedValueType = TAccountState;

			static TAccountState& Adapt(TAccountState& accountState) {
				return accountState;
			}
		};

	public:
		using ConstValueAdapter = ValueAdapter<const state::AccountState>;
		using MutableValueAdapter = ValueAdapter<state::AccountState>;

	// endregion

	// region secondary descriptors

	public:
		/// Describes a key-based interface on top of an account state cache.
		struct KeyLookupMapTypesDescriptor {
		public:
			using KeyType = Key;
			using ValueType = std::pair<Key, Address>;

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
			auto find(const KeyType& key) const {
				const auto* pPair = m_set1.find(key);
				return pPair ? utils::as_const(m_set2).find(pPair->second) : nullptr;
			}

			auto find(const KeyType& key) {
				const auto* pPair = m_set1.find(key);
				return pPair ? m_set2.find(pPair->second) : nullptr;
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
			using SetOneType = const KeyLookupMapTypes::BaseSetType;
			using SetTwoType = const PrimaryTypes::BaseSetType;
		};

		struct ComposableBaseSetDeltas {
			using SetOneType = const KeyLookupMapTypes::BaseSetDeltaType;
			using SetTwoType = PrimaryTypes::BaseSetDeltaType;
		};

	public:
		// in order to compose account state cache from multiple sets, define an aggregate set type

		struct BaseSetDeltaPointerType {
			PrimaryTypes::BaseSetDeltaPointerType pPrimary;
			KeyLookupMapTypes::BaseSetDeltaPointerType pKeyLookupMap;
		};

		struct BaseSetType {
		public:
			PrimaryTypes::BaseSetType Primary;
			KeyLookupMapTypes::BaseSetType KeyLookupMap;

		public:
			BaseSetDeltaPointerType rebase() {
				return { Primary.rebase(), KeyLookupMap.rebase() };
			}

			BaseSetDeltaPointerType rebaseDetached() const {
				return { Primary.rebaseDetached(), KeyLookupMap.rebaseDetached() };
			}

			void commit() {
				Primary.commit();
				KeyLookupMap.commit();
			}
		};
	};
}}
