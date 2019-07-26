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

namespace catapult { namespace state {

	/// Represents detailed information about a block difficulty
	/// including the block height and the block timestamp.
	struct BlockDifficultyInfo {

		/// Creates a default block difficulty info.
		constexpr BlockDifficultyInfo()
				: BlockDifficultyInfo(Height(0))
		{}

		/// Creates a block difficulty info from a \a height.
		constexpr explicit BlockDifficultyInfo(Height height)
				: BlockDifficultyInfo(height, Timestamp(0), Difficulty(0))
		{}

		/// Creates a block difficulty info from a \a height, a \a timestamp and a \a difficulty.
		constexpr BlockDifficultyInfo(Height height, Timestamp timestamp, Difficulty difficulty)
				: BlockHeight(height)
				, BlockTimestamp(timestamp)
				, BlockDifficulty(difficulty)
		{}

		/// Block height.
		Height BlockHeight;

		/// Block timestamp.
		Timestamp BlockTimestamp;

		/// Block difficulty.
		Difficulty BlockDifficulty;

		/// Returns \c true if this block difficulty info is less than \a rhs.
		constexpr bool operator<(const BlockDifficultyInfo& rhs) const {
			return BlockHeight < rhs.BlockHeight;
		}

		/// Returns \c true if this block difficulty info is equal to \a rhs.
		constexpr bool operator==(const BlockDifficultyInfo& rhs) const {
			return BlockHeight == rhs.BlockHeight;
		}

		/// Returns \c true if this block difficulty info is not equal to \a rhs.
		constexpr bool operator!=(const BlockDifficultyInfo& rhs) const {
			return BlockHeight != rhs.BlockHeight;
		}
	};
}}
