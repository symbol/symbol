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
#include "catapult/state/BlockStatistic.h"

namespace catapult {
	namespace cache {
		class BasicBlockStatisticCacheDelta;
		class BasicBlockStatisticCacheView;
		class BlockStatisticCache;
		class BlockStatisticCacheDelta;
		class BlockStatisticCacheView;

		template<typename TCache, typename TCacheDelta, typename TCacheKey>
		class ReadOnlySimpleCache;
	}
}

namespace catapult { namespace cache {

	/// Describes a block statistic cache.
	struct BlockStatisticCacheDescriptor {
	public:
		// key value types
		using KeyType = state::BlockStatistic;
		using ValueType = KeyType;

		// cache types
		using CacheType = BlockStatisticCache;
		using CacheDeltaType = BlockStatisticCacheDelta;
		using CacheViewType = BlockStatisticCacheView;

	public:
		/// Gets the key corresponding to \a blockStatistic.
		static const auto& GetKeyFromValue(const ValueType& blockStatistic) {
			return blockStatistic;
		}
	};

	/// Block statistic cache types.
	/// \note Mutable because time and difficulty can change for same height.
	struct BlockStatisticCacheTypes
			: public SingleSetCacheTypesAdapter<MutableOrderedMemorySetAdapter<BlockStatisticCacheDescriptor>, std::true_type> {
		using CacheReadOnlyType = ReadOnlySimpleCache<
			BasicBlockStatisticCacheView,
			BasicBlockStatisticCacheDelta,
			state::BlockStatistic>;

		/// Custom sub view options.
		struct Options {
			/// Block statistic history size.
			uint64_t HistorySize;
		};
	};

	/// Iterator range of block statistics.
	template<typename TIterator>
	class BlockStatisticRangeT {
	private:
		using IteratorType = TIterator;

	public:
		/// Creates a range around two iterators \a begin and \a end.
		BlockStatisticRangeT(const IteratorType& begin, const IteratorType& end)
				: m_begin(begin)
				, m_end(end)
		{}

	public:
		/// Gets an iterator that represents the first element.
		IteratorType begin() const {
			return m_begin;
		}

		/// Gets an iterator that represents one past the last element.
		IteratorType end() const {
			return m_end;
		}

	private:
		const IteratorType m_begin;
		const IteratorType m_end;
	};

	/// Iterator range of block statistics from a cache view.
	using BlockStatisticRange = BlockStatisticRangeT<
		BlockStatisticCacheTypes::PrimaryTypes::BaseSetType::SetType::MemorySetType::const_iterator>;
}}
