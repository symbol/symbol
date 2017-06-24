#pragma once
#include "BaseValue.h"
#include <unordered_set>
#include <stdint.h>

namespace catapult { namespace utils {

	struct ShortHash_tag {};
	using ShortHash = BaseValue<uint32_t, ShortHash_tag>;

	// Trivial hash function for ShortHash
	struct ShortHashHasher {
		size_t operator()(const ShortHash& shortHash) const {
			return shortHash.unwrap();
		}
	};

	using ShortHashesSet = std::unordered_set<utils::ShortHash, utils::ShortHashHasher>;
}}
