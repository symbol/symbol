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
#include "catapult/types.h"
#include <iosfwd>

namespace catapult { namespace model {

#pragma pack(push, 1)

	/// Pinned voting key.
	struct PinnedVotingKey {
	public:
		static constexpr auto Size = catapult::VotingKey::Size + 2 * sizeof(FinalizationPoint);

	public:
		/// Voting key.
		catapult::VotingKey VotingKey;

		/// Start finalization point.
		FinalizationPoint StartPoint;

		/// End finalization point.
		FinalizationPoint EndPoint;

	public:
		/// Returns \c true if this root voting key is equal to \a rhs.
		bool operator==(const PinnedVotingKey& rhs) const {
			return VotingKey == rhs.VotingKey && StartPoint == rhs.StartPoint && EndPoint == rhs.EndPoint;
		}

		/// Returns \c true if this root voting key is not equal to \a rhs.
		bool operator!=(const PinnedVotingKey& rhs) const {
			return !(*this == rhs);
		}

		/// Insertion operator for outputting \a key to \a out.
		friend std::ostream& operator<<(std::ostream& out, const PinnedVotingKey& key) {
			out << key.VotingKey << " @ [" << key.StartPoint << ", " << key.EndPoint << "]";
			return out;
		}
	};

#pragma pack(pop)
}}
