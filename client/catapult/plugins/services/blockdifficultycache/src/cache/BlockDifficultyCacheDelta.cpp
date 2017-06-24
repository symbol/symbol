#include "BlockDifficultyCacheDelta.h"
#include "catapult/exceptions.h"

namespace catapult { namespace cache {

	BasicBlockDifficultyCacheDelta::BasicBlockDifficultyCacheDelta(
			const block_difficulty_cache_types::BaseSetDeltaPointerType& pDelta,
			const ValueType* const pFirstOriginalElement,
			uint64_t difficultyHistorySize)
			: m_pOrderedDelta(pDelta)
			, m_difficultyHistorySize(difficultyHistorySize)
			// note: nullptr == pFirstOriginalElement means we just started populating the cache,
			//       it cannot happen due to a rollback because the nemesis block cannot be rolled back.
			, m_startHeight(nullptr != pFirstOriginalElement ? pFirstOriginalElement->BlockHeight : Height(1))
	{}

	void BasicBlockDifficultyCacheDelta::prune(Height height) {
		auto heightDifference = Height(m_difficultyHistorySize);
		if (height > heightDifference)
			m_pruningBoundary = deltaset::PruningBoundary<ValueType>(ValueType(height - heightDifference));
	}

	void BasicBlockDifficultyCacheDelta::checkInsert(Height height) {
		// insert is only allowed if either
		// - the set is empty
		// - the height is one larger than the largest height in the set
		if (m_pOrderedDelta->empty()) {
			m_startHeight = height;
			return;
		}

		if (nextHeight() != height)
			CATAPULT_THROW_INVALID_ARGUMENT_1("insertion of entity with unexpected height: ", height);
	}

	void BasicBlockDifficultyCacheDelta::checkRemove(Height height) const {
		// remove is only allowed if both
		// - the set is non-empty
		// - the height is equal to the largest height in the set
		if (m_pOrderedDelta->empty())
			CATAPULT_THROW_RUNTIME_ERROR("remove called on empty cache");

		if (nextHeight() != height + Height(1))
			CATAPULT_THROW_INVALID_ARGUMENT_1("removal of entity with unexpected height: ", height);
	}

	Height BasicBlockDifficultyCacheDelta::nextHeight() const {
		return Height(m_startHeight.unwrap() + size());
	}
}}
