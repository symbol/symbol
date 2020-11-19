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
#include "catapult/model/Block.h"
#include <iosfwd>

namespace catapult { namespace state {

	/// Represents detailed information about a block statistic
	/// including the block height and the block timestamp.
	struct BlockStatistic {
	public:
		/// Creates a default block statistic.
		constexpr BlockStatistic() : BlockStatistic(catapult::Height(0))
		{}

		/// Creates a block statistic from \a height.
		constexpr explicit BlockStatistic(catapult::Height height)
				: BlockStatistic(height, catapult::Timestamp(0), catapult::Difficulty(0), BlockFeeMultiplier(0))
		{}

		/// Creates a block statistic from \a block.
		constexpr explicit BlockStatistic(const model::Block& block)
				: BlockStatistic(block.Height, block.Timestamp, block.Difficulty, block.FeeMultiplier)
		{}

		/// Creates a block statistic from \a height, \a timestamp, \a difficulty and \a feeMultiplier.
		constexpr BlockStatistic(
				catapult::Height height,
				catapult::Timestamp timestamp,
				catapult::Difficulty difficulty,
				BlockFeeMultiplier feeMultiplier)
				: Height(height)
				, Timestamp(timestamp)
				, Difficulty(difficulty)
				, FeeMultiplier(feeMultiplier)
		{}

	public:
		/// Block height.
		catapult::Height Height;

		/// Block timestamp.
		catapult::Timestamp Timestamp;

		/// Block difficulty.
		catapult::Difficulty Difficulty;

		/// Block fee multiplier.
		BlockFeeMultiplier FeeMultiplier;

	public:
		/// Returns \c true if this block statistic is less than \a rhs.
		constexpr bool operator<(const BlockStatistic& rhs) const {
			return Height < rhs.Height;
		}

		/// Returns \c true if this block statistic is equal to \a rhs.
		constexpr bool operator==(const BlockStatistic& rhs) const {
			return Height == rhs.Height;
		}

		/// Returns \c true if this block statistic is not equal to \a rhs.
		constexpr bool operator!=(const BlockStatistic& rhs) const {
			return Height != rhs.Height;
		}
	};

	/// Insertion operator for outputting \a statistic to \a out.
	std::ostream& operator<<(std::ostream& out, const BlockStatistic& statistic);
}}
