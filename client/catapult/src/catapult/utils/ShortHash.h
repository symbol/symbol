#pragma once
#include "BaseValue.h"
#include "catapult/types.h"
#include <unordered_set>

namespace catapult { namespace utils {

	struct ShortHash_tag {};
	using ShortHash = BaseValue<uint32_t, ShortHash_tag>;

	// Trivial hash function for ShortHash
	struct ShortHashHasher {
		size_t operator()(const ShortHash& shortHash) const {
			return shortHash.unwrap();
		}
	};

	// An unordered set of short hashes.
	using ShortHashesSet = std::unordered_set<ShortHash, ShortHashHasher>;

	/// Gets the short hash corresponding to \a hash.
	inline ShortHash ToShortHash(const Hash256& hash) {
		return reinterpret_cast<const ShortHash&>(*hash.data());
	}
}}
