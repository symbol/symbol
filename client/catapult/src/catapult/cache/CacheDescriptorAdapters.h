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

#pragma once
#include "CacheConfiguration.h"
#include "catapult/cache_db/CacheDatabase.h"
#include "catapult/cache_db/UpdateSet.h"
#include "catapult/deltaset/BaseSet.h"
#include "catapult/deltaset/ConditionalContainer.h"
#include "catapult/deltaset/OrderedSet.h"
#include <unordered_map>

namespace catapult { namespace cache {

	namespace detail {
		/// Defines cache types for an unordered map based cache.
		template<typename TElementTraits, typename TDescriptor, typename TValueHasher>
		struct UnorderedMapAdapter {
		private:
			struct DescriptorAdapter {
			public:
				using KeyType = typename TDescriptor::KeyType;
				using ValueType = typename TDescriptor::ValueType;
				using StorageType = std::pair<const KeyType, ValueType>;
				using Serializer = typename TDescriptor::Serializer;

				static constexpr auto GetKeyFromValue = TDescriptor::GetKeyFromValue;

				static constexpr auto& ToKey(const StorageType& element) {
					return element.first;
				}

				static constexpr auto& ToValue(const StorageType& element) {
					return element.second;
				}

				static auto ToStorage(const ValueType& value) {
					return StorageType(TDescriptor::GetKeyFromValue(value), value);
				}
			};

			using StorageMapType = CacheContainerView<DescriptorAdapter>;
			using MemoryMapType = std::unordered_map<typename TDescriptor::KeyType, typename TDescriptor::ValueType, TValueHasher>;

			struct Converter {
				static constexpr auto ToKey = TDescriptor::GetKeyFromValue;
			};

			// workaround for VS truncation
			using MapStorageTraits = deltaset::MapStorageTraits<
				deltaset::ConditionalContainer<
					deltaset::MapKeyTraits<MemoryMapType>,
					StorageMapType,
					MemoryMapType
				>,
				Converter,
				MemoryMapType
			>;

			struct StorageTraits : public MapStorageTraits {};

		public:
			/// Base set type.
			using BaseSetType = deltaset::BaseSet<TElementTraits, StorageTraits>;

			/// Base set delta type.
			using BaseSetDeltaType = typename BaseSetType::DeltaType;

			/// Base set delta pointer type.
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
		/// Defines cache types for an ordered, memory backed set based cache.
		template<typename TElementTraits>
		struct OrderedMemorySetAdapter {
		private:
			using ElementType = std::remove_const_t<typename TElementTraits::ElementType>;

			class StorageSetType : public deltaset::detail::OrderedSetType<TElementTraits> {
			public:
				StorageSetType(CacheDatabase&, size_t)
				{}
			};

			using MemorySetType = std::set<ElementType>;

			// workaround for VS truncation
			using SetStorageTraits = deltaset::SetStorageTraits<
				deltaset::ConditionalContainer<
					deltaset::SetKeyTraits<MemorySetType>,
					StorageSetType,
					MemorySetType
				>,
				MemorySetType
			>;

			struct StorageTraits : public SetStorageTraits {};

		public:
			/// Base set type.
			using BaseSetType = deltaset::OrderedSet<TElementTraits, StorageTraits>;

			/// Base set delta type.
			using BaseSetDeltaType = typename BaseSetType::DeltaType;

			/// Base set delta pointer type.
			using BaseSetDeltaPointerType = std::shared_ptr<BaseSetDeltaType>;
		};
	}

	/// Defines cache types for an ordered, mutable, memory backed set based cache.
	template<typename TDescriptor>
	using MutableOrderedMemorySetAdapter = detail::OrderedMemorySetAdapter<
		deltaset::MutableTypeTraits<typename TDescriptor::ValueType>>;

	/// Defines cache types for an ordered, immutable, memory backed set based cache.
	template<typename TDescriptor>
	using ImmutableOrderedMemorySetAdapter = detail::OrderedMemorySetAdapter<
		deltaset::ImmutableTypeTraits<typename TDescriptor::ValueType>>;

	namespace detail {
		/// Defines cache types for an ordered set based cache.
		template<typename TElementTraits, typename TDescriptor>
		struct OrderedSetAdapter {
		private:
			struct DescriptorAdapter {
			public:
				using KeyType = typename TDescriptor::KeyType;
				using ValueType = typename TDescriptor::ValueType;
				using StorageType = typename TDescriptor::KeyType;
				using Serializer = typename TDescriptor::Serializer;

				static constexpr auto GetKeyFromValue = TDescriptor::GetKeyFromValue;

				static constexpr auto& ToKey(const StorageType& element) {
					return element;
				}

				static constexpr auto& ToValue(const StorageType& element) {
					return element;
				}

				static constexpr auto& ToStorage(const ValueType& value) {
					return value;
				}
			};

			using ElementType = std::remove_const_t<typename TElementTraits::ElementType>;
			using StorageSetType = CacheContainerView<DescriptorAdapter>;
			using MemorySetType = std::set<ElementType>;

			// workaround for VS truncation
			using SetStorageTraits = deltaset::SetStorageTraits<
				deltaset::ConditionalContainer<
					deltaset::SetKeyTraits<MemorySetType>,
					StorageSetType,
					MemorySetType
				>,
				MemorySetType
			>;

			struct StorageTraits : public SetStorageTraits {};

		public:
			/// Base set type.
			using BaseSetType = deltaset::OrderedSet<TElementTraits, StorageTraits>;

			/// Base set delta type.
			using BaseSetDeltaType = typename BaseSetType::DeltaType;

			/// Base set delta pointer type.
			using BaseSetDeltaPointerType = std::shared_ptr<BaseSetDeltaType>;
		};
	}

	/// Defines cache types for an ordered mutable set based cache.
	template<typename TDescriptor>
	using MutableOrderedSetAdapter = detail::OrderedSetAdapter<
		deltaset::MutableTypeTraits<typename TDescriptor::ValueType>,
		TDescriptor>;

	/// Defines cache types for an ordered immutable set based cache.
	template<typename TDescriptor>
	using ImmutableOrderedSetAdapter = detail::OrderedSetAdapter<
		deltaset::ImmutableTypeTraits<typename TDescriptor::ValueType>,
		TDescriptor>;
}}
