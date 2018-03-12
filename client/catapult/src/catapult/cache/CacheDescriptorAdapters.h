#pragma once
#include "catapult/deltaset/BaseSet.h"
#include "catapult/deltaset/OrderedSet.h"
#include <unordered_map>

namespace catapult { namespace cache {

	namespace detail {
		/// Defines cache types for an unordered map based cache.
		template<typename TElementTraits, typename TDescriptor, typename TValueHasher>
		struct UnorderedMapAdapter {
		private:
			using UnorderedMapType = std::unordered_map<typename TDescriptor::KeyType, typename TDescriptor::ValueType, TValueHasher>;

			struct Converter {
				static constexpr auto ToKey = TDescriptor::GetKeyFromValue;
			};

		public:
			/// The base set type.
			using BaseSetType = deltaset::BaseSet<TElementTraits, deltaset::MapStorageTraits<UnorderedMapType, Converter>>;

			/// The base set delta type.
			using BaseSetDeltaType = typename BaseSetType::DeltaType;

			/// The base set delta pointer type.
			using BaseSetDeltaPointerType = std::shared_ptr<BaseSetDeltaType>;
		};
	}

	/// Defines cache types for an unordered mutable map based cache.
	template<typename TDescriptor, typename TValueHasher = std::hash<typename TDescriptor::KeyType>>
	using MutableUnorderedMapAdapter = detail::UnorderedMapAdapter<
		deltaset::MutableTypeTraits<typename TDescriptor::ValueType>,
		TDescriptor,
		TValueHasher>;

	/// Defines cache types for an unordered immutable map based cache.
	template<typename TDescriptor, typename TValueHasher = std::hash<typename TDescriptor::KeyType>>
	using ImmutableUnorderedMapAdapter = detail::UnorderedMapAdapter<
		deltaset::ImmutableTypeTraits<typename TDescriptor::ValueType>,
		TDescriptor,
		TValueHasher>;

	namespace detail {
		/// Defines cache types for an ordered set based cache.
		template<typename TElementTraits>
		struct OrderedSetAdapter {
			/// The base set type.
			using BaseSetType = deltaset::OrderedSet<TElementTraits>;

			/// The base set delta type.
			using BaseSetDeltaType = typename BaseSetType::DeltaType;

			/// The base set delta pointer type.
			using BaseSetDeltaPointerType = std::shared_ptr<BaseSetDeltaType>;
		};
	}

	/// Defines cache types for an ordered mutable set based cache.
	template<typename TDescriptor>
	using MutableOrderedSetAdapter = detail::OrderedSetAdapter<deltaset::MutableTypeTraits<typename TDescriptor::ValueType>>;

	/// Defines cache types for an ordered immutable set based cache.
	template<typename TDescriptor>
	using ImmutableOrderedSetAdapter = detail::OrderedSetAdapter<deltaset::ImmutableTypeTraits<typename TDescriptor::ValueType>>;
}}
