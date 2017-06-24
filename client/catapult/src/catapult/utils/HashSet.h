#pragma once
#include "Hashers.h"
#include "catapult/types.h"
#include <unordered_set>

namespace catapult { namespace utils {

	/// A hash set.
	using HashSet = std::unordered_set<Hash256, ArrayHasher<Hash256>>;

	/// A key set.
	/// \note Key and Hash256 have same underlying type.
	using KeySet = HashSet;

	/// Functor for hashing a hash pointer.
	struct Hash256PointerHasher {
		size_t operator()(const Hash256* pHash) const {
			return ArrayHasher<Hash256>()(*pHash);
		}
	};

	/// Functor for comparing two hash pointers.
	struct Hash256PointerEquality {
		bool operator()(const Hash256* pLhs, const Hash256* pRhs) const {
			return *pLhs == *pRhs;
		}
	};

	/// A hash pointer set.
	using HashPointerSet = std::unordered_set<const Hash256*, Hash256PointerHasher, Hash256PointerEquality>;

	/// A key pointer set.
	/// \note Key and Hash256 have same underlying type.
	using KeyPointerSet = HashPointerSet;
}}
