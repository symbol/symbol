#include "BlockDifficultyCacheView.h"
#include "catapult/exceptions.h"

namespace catapult { namespace cache {

	BasicBlockDifficultyCacheView::BasicBlockDifficultyCacheView(
			const BlockDifficultyCacheTypes::BaseSetType& difficultyInfos,
			const BlockDifficultyCacheTypes::Options&)
			: SizeMixin<BlockDifficultyCacheTypes::BaseSetType>(difficultyInfos)
			, ContainsMixin<BlockDifficultyCacheTypes::BaseSetType, BlockDifficultyCacheDescriptor>(difficultyInfos)
			, SetIterationMixin<BlockDifficultyCacheTypes::BaseSetType, BlockDifficultyCacheDescriptor>(difficultyInfos)
			, m_difficultyInfos(difficultyInfos)
	{}

	namespace {
		constexpr state::BlockDifficultyInfo CreateFromHeight(Height height) {
			return state::BlockDifficultyInfo(height);
		}
	}

	DifficultyInfoRange BasicBlockDifficultyCacheView::difficultyInfos(Height height, size_t count) const {
		if (m_difficultyInfos.empty())
			// note: this should not happen since the nemesis block is available from the beginning
			CATAPULT_THROW_RUNTIME_ERROR("block difficulty cache is empty")

		if (Height(0) == height || 0 == count)
			CATAPULT_THROW_INVALID_ARGUMENT("specified height or count out of range");

		const auto last = CreateFromHeight(height);
		auto lastIter = m_difficultyInfos.findIterator(last);
		if (m_difficultyInfos.end() == lastIter)
			CATAPULT_THROW_INVALID_ARGUMENT_1("element with specified height not found", height);

		const auto& firstSetElement = *m_difficultyInfos.begin();
		const auto first = height.unwrap() - firstSetElement.BlockHeight.unwrap() < count - 1
			? CreateFromHeight(firstSetElement.BlockHeight)
			: CreateFromHeight(height - Height(count - 1));
		auto firstIter = m_difficultyInfos.findIterator(first);

		return DifficultyInfoRange(firstIter, ++lastIter);
	}
}}
