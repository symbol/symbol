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
#include "catapult/cache/CacheDescriptorAdapters.h"
#include "catapult/cache/SingleSetCacheTypesAdapter.h"
#include "catapult/state/BlockDifficultyInfo.h"

namespace catapult {
	namespace cache {
		class BasicBlockDifficultyCacheDelta;
		class BasicBlockDifficultyCacheView;
		class BlockDifficultyCache;
		class BlockDifficultyCacheDelta;
		class BlockDifficultyCacheView;

		template<typename TCache, typename TCacheDelta, typename TKey>
		class ReadOnlySimpleCache;
	}
}

namespace catapult { namespace cache {

	/// Describes a block difficulty cache.
	struct BlockDifficultyCacheDescriptor {
	public:
		// key value types
		using KeyType = state::BlockDifficultyInfo;
		using ValueType = KeyType;

		// cache types
		using CacheType = BlockDifficultyCache;
		using CacheDeltaType = BlockDifficultyCacheDelta;
		using CacheViewType = BlockDifficultyCacheView;

	public:
		/// Gets the key corresponding to \a blockDifficultyInfo.
		static const auto& GetKeyFromValue(const ValueType& blockDifficultyInfo) {
			return blockDifficultyInfo;
		}
	};

	/// Block difficulty cache types.
	/// \note Mutable because time and difficulty can change for same height.
	struct BlockDifficultyCacheTypes
			: public SingleSetCacheTypesAdapter<MutableOrderedSetAdapter<BlockDifficultyCacheDescriptor>, std::true_type> {
		using CacheReadOnlyType = ReadOnlySimpleCache<
			BasicBlockDifficultyCacheView,
			BasicBlockDifficultyCacheDelta,
			state::BlockDifficultyInfo>;

		/// Custom sub view options.
		struct Options {
			/// Difficulty history size.
			uint64_t DifficultyHistorySize;
		};
	};

	/// A range of block difficulty infos.
	class DifficultyInfoRange {
	private:
		using IteratorType = BlockDifficultyCacheTypes::PrimaryTypes::BaseSetType::SetType::MemorySetType::const_iterator;

	public:
		/// Creates a range around two iterators \a begin and \a end.
		DifficultyInfoRange(const IteratorType& begin, const IteratorType& end)
				: m_begin(begin)
				, m_end(end)
		{}

	public:
		/// Returns an iterator that represents the first element.
		IteratorType begin() const {
			return m_begin;
		}

		/// Returns an iterator that represents one past the last element.
		IteratorType end() const {
			return m_end;
		}

	private:
		const IteratorType m_begin;
		const IteratorType m_end;
	};
}}
