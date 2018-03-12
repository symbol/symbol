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
		constexpr explicit BlockDifficultyInfo(Height height, Timestamp timestamp, Difficulty difficulty)
				: BlockHeight(height)
				, BlockTimestamp(timestamp)
				, BlockDifficulty(difficulty)
		{}

		/// The block height.
		Height BlockHeight;

		/// The block timestamp.
		Timestamp BlockTimestamp;

		/// The block difficulty
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
