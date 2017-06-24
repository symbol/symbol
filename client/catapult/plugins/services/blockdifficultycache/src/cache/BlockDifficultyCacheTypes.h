#pragma once
#include "catapult/deltaset/OrderedSet.h"
#include "catapult/state/BlockDifficultyInfo.h"

namespace catapult {
	namespace cache {
		class BasicBlockDifficultyCacheDelta;
		class BasicBlockDifficultyCacheView;

		template<typename TCache, typename TCacheDelta, typename TKey>
		class ReadOnlySimpleCache;
	}
}

namespace catapult { namespace cache {
	namespace block_difficulty_cache_types {
		/// The cache value type.
		using ValueType = state::BlockDifficultyInfo;

		/// The entity traits.
		using EntityTraits = deltaset::MutableTypeTraits<ValueType>;

		/// The base set type.
		using BaseSetType = deltaset::OrderedSet<EntityTraits>;

		/// The underlying set type.
		using DifficultySet = BaseSetType::SetType;

		/// The underlying set type iterator.
		using DifficultySetIterator = DifficultySet::const_iterator;

		/// A pointer to the base set delta type.
		using BaseSetDeltaPointerType = std::shared_ptr<BaseSetType::DeltaType>;

		/// A read-only view of a block difficulty cache.
		using CacheReadOnlyType = ReadOnlySimpleCache<BasicBlockDifficultyCacheView, BasicBlockDifficultyCacheDelta, ValueType>;
	}

	/// A range of block difficulty infos.
	class DifficultyInfoRange {
	private:
		using IteratorType = block_difficulty_cache_types::DifficultySetIterator;

	public:
		/// Creates a range around two iterators \a begin and \a end.
		DifficultyInfoRange(const IteratorType& begin, const IteratorType& end)
				: m_begin(begin)
				, m_end(end)
		{}

	public:
		/// Returns an iterator that represents the first entity.
		IteratorType begin() const {
			return m_begin;
		}

		/// Returns an iterator that represents one past the last entity.
		IteratorType end() const {
			return m_end;
		}

	private:
		const IteratorType m_begin;
		const IteratorType m_end;
	};
}}
