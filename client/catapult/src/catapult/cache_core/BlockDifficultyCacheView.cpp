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

#include "BlockDifficultyCacheView.h"
#include "catapult/deltaset/BaseSetIterationView.h"
#include "catapult/exceptions.h"

namespace catapult { namespace cache {

	BasicBlockDifficultyCacheView::BasicBlockDifficultyCacheView(
			const BlockDifficultyCacheTypes::BaseSets& difficultyInfoSets,
			const BlockDifficultyCacheTypes::Options&)
			: BlockDifficultyCacheViewMixins::Size(difficultyInfoSets.Primary)
			, BlockDifficultyCacheViewMixins::Contains(difficultyInfoSets.Primary)
			, BlockDifficultyCacheViewMixins::Iteration(difficultyInfoSets.Primary)
			, m_difficultyInfos(difficultyInfoSets.Primary)
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

		auto iterableDifficultyInfos = MakeIterableView(m_difficultyInfos);
		const auto last = CreateFromHeight(height);
		auto lastIter = iterableDifficultyInfos.findIterator(last);
		if (iterableDifficultyInfos.end() == lastIter)
			CATAPULT_THROW_INVALID_ARGUMENT_1("element with specified height not found", height);

		const auto& firstSetElement = *iterableDifficultyInfos.begin();
		const auto first = height.unwrap() - firstSetElement.BlockHeight.unwrap() < count - 1
				? CreateFromHeight(firstSetElement.BlockHeight)
				: CreateFromHeight(height - Height(count - 1));
		auto firstIter = iterableDifficultyInfos.findIterator(first);

		return DifficultyInfoRange(firstIter, ++lastIter);
	}
}}
