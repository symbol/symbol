#pragma once
#include "catapult/model/EntityRange.h"
#include "catapult/utils/ShortHash.h"
#include <unordered_map>

namespace catapult { namespace cache {

#pragma pack(push, 1)

	/// A unique identifier for a partial transaction.
	struct ShortHashPair {
	public:
		/// The transaction short hash.
		utils::ShortHash TransactionShortHash;

		/// The cosignatures short hash.
		utils::ShortHash CosignaturesShortHash;

	public:
		/// Returns \c true if this short hash pair is equal to \a rhs.
		constexpr bool operator==(const ShortHashPair& rhs) const {
			return TransactionShortHash == rhs.TransactionShortHash && CosignaturesShortHash == rhs.CosignaturesShortHash;
		}

		/// Returns \c true if this short hash pair is not equal to \a rhs.
		constexpr bool operator!=(const ShortHashPair& rhs) const {
			return !(*this == rhs);
		}
	};

#pragma pack(pop)

	/// An entity range composed of short hash pairs.
	using ShortHashPairRange = model::EntityRange<ShortHashPair>;

	/// A map composed of short hash pairs where the key is the transaction short hash and the value is the cosignatures short hash.
	using ShortHashPairMap = std::unordered_map<utils::ShortHash, utils::ShortHash, utils::ShortHashHasher>;
}}
