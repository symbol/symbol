#pragma once
#include "Hashers.h"
#include "catapult/types.h"
#include <unordered_map>
#include <unordered_set>

namespace catapult { namespace utils {

	/// Functor for hashing an array pointer.
	template<typename TArray>
	struct ArrayPointerHasher {
		size_t operator()(const TArray* pArray) const {
			return ArrayHasher<TArray>()(*pArray);
		}
	};

	/// Functor for comparing two array pointers.
	template<typename TArray>
	struct ArrayPointerEquality {
		bool operator()(const TArray* pLhs, const TArray* pRhs) const {
			return *pLhs == *pRhs;
		}
	};

	/// A set of arrays.
	template<typename TArray>
	using ArraySet = std::unordered_set<TArray, ArrayHasher<TArray>>;

	/// A set of array pointers.
	template<typename TArray>
	using ArrayPointerSet = std::unordered_set<const TArray*, ArrayPointerHasher<TArray>, ArrayPointerEquality<TArray>>;

	/// A map of array pointers to flags.
	template<typename TArray>
	using ArrayPointerFlagMap = std::unordered_map<const TArray*, bool, ArrayPointerHasher<TArray>, ArrayPointerEquality<TArray>>;

	// region well known array sets

	/// A hash set.
	using HashSet = ArraySet<Hash256>;

	/// A key set.
	using KeySet = ArraySet<Key>;

	/// A hash pointer set.
	using HashPointerSet = ArrayPointerSet<Hash256>;

	/// A key pointer set.
	using KeyPointerSet = ArrayPointerSet<Key>;

	// endregion
}}
