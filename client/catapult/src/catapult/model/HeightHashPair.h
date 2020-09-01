/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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
#include "EntityRange.h"
#include "catapult/types.h"

namespace catapult { namespace model {

	/// Height hash pair.
	struct HeightHashPair {
	public:
		static constexpr auto Size = sizeof(catapult::Height) + Hash256::Size;

	public:
		/// Height.
		catapult::Height Height;

		/// Hash.
		Hash256 Hash;

	public:
		/// Returns \c true if this pair is equal to \a rhs.
		constexpr bool operator==(const HeightHashPair& rhs) const {
			return Height == rhs.Height && Hash == rhs.Hash;
		}

		/// Returns \c true if this pair is not equal to \a rhs.
		constexpr bool operator!=(const HeightHashPair& rhs) const {
			return !(*this == rhs);
		}

		/// Insertion operator for outputting \a pair to \a out.
		friend std::ostream& operator<<(std::ostream& out, const HeightHashPair& pair) {
			out << pair.Hash << " @ " << pair.Height;
			return out;
		}
	};
}}
