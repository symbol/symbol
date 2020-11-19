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
#include <memory>

namespace catapult { namespace deltaset {

	// region key + storage traits

	// KeyType => the data key
	// ValueType => the data value (this is assumed to be convertible to KeyType)
	// StorageType => the underlying storage (e.g. pair) composed of both key and value

	/// Key-related traits for stl set types.
	template<typename TSet>
	struct SetKeyTraits {
		using KeyType = typename TSet::value_type;
		using ValueType = typename TSet::value_type;
		using StorageType = typename TSet::value_type;

		/// Converts a storage type (\a element) to a key.
		static constexpr const KeyType& ToKey(const StorageType& element) {
			return element;
		}
	};

	/// Key-related traits for stl map types.
	template<typename TMap>
	struct MapKeyTraits {
		using KeyType = typename TMap::key_type;
		using ValueType = typename TMap::mapped_type;
		using StorageType = typename TMap::value_type;

		/// Converts a storage type (\a element) to a key.
		static constexpr const KeyType& ToKey(const StorageType& element) {
			return element.first;
		}
	};

	/// Base set compatible traits for stl set types.
	template<typename TSet, typename TMemorySetType = TSet>
	struct SetStorageTraits {
		using SetType = TSet;
		using MemorySetType = TMemorySetType;

		using KeyTraits = SetKeyTraits<MemorySetType>;
		using KeyType = typename KeyTraits::KeyType;
		using ValueType = typename KeyTraits::ValueType;
		using StorageType = typename KeyTraits::StorageType;

		/// Values contained in set cannot be modified because they are hashed in native container.
		static constexpr bool AllowsNativeValueModification = false;

		/// Converts a storage type (\a element) to a key.
		static constexpr const KeyType& ToKey(const StorageType& element) {
			return KeyTraits::ToKey(element);
		}

		/// Converts a value type (\a value) to a storage type.
		static constexpr const StorageType& ToStorage(const ValueType& value) {
			return value;
		}

		/// Converts \a key to a storage type.
		template<typename TIterator>
		static const StorageType& ToStorage(const KeyType& key, TIterator&&) {
			return key;
		}

		/// Converts a storage type (\a element) to a value type.
		static constexpr const ValueType& ToValue(const StorageType& element) {
			return element;
		}
	};

	/// Base set compatible traits for stl map types.
	template<typename TMap, typename TElementToKeyConverter, typename TMemoryMapType = TMap>
	struct MapStorageTraits {
		using SetType = TMap;
		using MemorySetType = TMemoryMapType;

		using KeyTraits = MapKeyTraits<MemorySetType>;
		using KeyType = typename KeyTraits::KeyType;
		using ValueType = typename KeyTraits::ValueType;
		using StorageType = typename KeyTraits::StorageType;

		/// Map values can be modified because they are not hashed in native container.
		static constexpr bool AllowsNativeValueModification = true;

		/// Converts a storage type (\a element) to a key.
		static constexpr const KeyType& ToKey(const StorageType& element) {
			return KeyTraits::ToKey(element);
		}

		/// Converts a value type (\a value) to a key.
		static constexpr KeyType ToKey(const ValueType& value) {
			return TElementToKeyConverter::ToKey(value);
		}

		/// Converts a value type (\a value) to a storage type.
		static constexpr StorageType ToStorage(const ValueType& value) {
			return std::make_pair(ToKey(value), value);
		}

		/// Converts \a iter to a storage type.
		template<typename TIterator>
		static const StorageType& ToStorage(const KeyType&, TIterator&& iter) {
			return *iter;
		}

		/// Converts a storage type (\a element) to a value type.
		static constexpr const ValueType& ToValue(const StorageType& element) {
			return element.second;
		}

		/// Converts a storage type (\a element) to a value type.
		static constexpr ValueType& ToValue(StorageType& element) {
			return element.second;
		}
	};

	// endregion

	// region mutability traits

	// mutability tagging allows BaseSet to optimize for immutable values that can never be modified
	// in contrast, mutable values have copy-on-write semantics

	/// Tag that indicates a type is mutable.
	struct MutableTypeTag {};

	/// Traits used for describing a mutable type.
	template<typename TElement>
	struct MutableTypeTraits {
		using ElementType = TElement;
		using MutabilityTag = MutableTypeTag;

		static constexpr TElement Copy(const TElement* pElement) {
			return *pElement;
		}
	};

	/// Tag that indicates a type is immutable.
	struct ImmutableTypeTag {};

	/// Traits used for describing an immutable type.
	template<typename TElement>
	struct ImmutableTypeTraits {
		using ElementType = const TElement;
		using MutabilityTag = ImmutableTypeTag;
	};

	// endregion

	// region find traits

	// used to find values, ensuring that values stored in stl set-based containers are always exposed as const
	// (because they're not modifiable)

	/// Traits for customizing the behavior of find depending on element type.
	template<typename T, bool AllowsNativeValueModification>
	struct FindTraitsT {
		using ConstResultType = const T*;
		using ResultType = const T*;

		static constexpr ResultType ToResult(const T& value) {
			return &value;
		}
	};

	template<typename T>
	struct FindTraitsT<T, true> {
		using ConstResultType = const T*;
		using ResultType = T*;

		// this needs to be a template in order to allow T to be const (immutable)
		template<typename TValue>
		static constexpr auto ToResult(TValue& value) {
			return &value;
		}
	};

	// endregion
}}
