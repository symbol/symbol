#pragma once
#include "BlockDifficultyCacheTypes.h"
#include "catapult/cache/ReadOnlySimpleCache.h"
#include "catapult/cache/ReadOnlyViewSupplier.h"
#include "catapult/utils/NonCopyable.h"
#include "catapult/utils/SpinReaderWriterLock.h"

namespace catapult { namespace cache {

	/// Basic view on top of the block difficulty cache.
	class BasicBlockDifficultyCacheView : public utils::MoveOnly {
	public:
		using ReadOnlyView = block_difficulty_cache_types::CacheReadOnlyType;

	public:
		/// Creates a view around \a difficultyInfos.
		explicit BasicBlockDifficultyCacheView(const block_difficulty_cache_types::BaseSetType& difficultyInfos)
				: m_difficultyInfos(difficultyInfos)
		{}

	public:
		/// Gets the size of the cache.
		auto size() const {
			return m_difficultyInfos.size();
		}

		/// Searches for the given block difficulty \a info in the cache.
		/// Returns a \c true if it is found or \c false if it is not found.
		bool contains(const state::BlockDifficultyInfo& info) const {
			return m_difficultyInfos.contains(info);
		}

		/// Gets a range object that spans \a count block difficulty infos starting at the specified \a height.
		DifficultyInfoRange difficultyInfos(Height height, size_t count) const;

	public:
		/// Returns a const iterator to the first element of the underlying set.
		auto cbegin() const {
			return m_difficultyInfos.cbegin();
		}

		/// Returns a const iterator to the element following the last element of the underlying set.
		auto cend() const {
			return m_difficultyInfos.cend();
		}

	private:
		const block_difficulty_cache_types::BaseSetType& m_difficultyInfos;
	};

	/// View on top of the block difficulty cache.
	class BlockDifficultyCacheView : public ReadOnlyViewSupplier<BasicBlockDifficultyCacheView> {
	public:
		/// Creates a view around \a difficultyInfos.
		explicit BlockDifficultyCacheView(const block_difficulty_cache_types::BaseSetType& difficultyInfos)
				: ReadOnlyViewSupplier(difficultyInfos)
		{}
	};
}}
