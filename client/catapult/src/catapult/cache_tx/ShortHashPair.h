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
#include "catapult/model/EntityRange.h"
#include "catapult/utils/ShortHash.h"
#include <unordered_map>

namespace catapult { namespace cache {

#pragma pack(push, 1)

	/// Unique identifier for a partial transaction.
	struct ShortHashPair {
	public:
		/// Transaction short hash.
		utils::ShortHash TransactionShortHash;

		/// Cosignatures short hash.
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

	/// Entity range composed of short hash pairs.
	using ShortHashPairRange = model::EntityRange<ShortHashPair>;

	/// Map composed of short hash pairs where the key is the transaction short hash and the value is the cosignatures short hash.
	using ShortHashPairMap = std::unordered_map<utils::ShortHash, utils::ShortHash, utils::ShortHashHasher>;
}}
