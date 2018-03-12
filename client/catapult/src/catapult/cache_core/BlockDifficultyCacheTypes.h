#pragma once
#include "catapult/cache/CacheDescriptorAdapters.h"
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
	struct BlockDifficultyCacheTypes : public MutableOrderedSetAdapter<BlockDifficultyCacheDescriptor> {
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
		using IteratorType = BlockDifficultyCacheTypes::BaseSetType::SetType::const_iterator;

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
