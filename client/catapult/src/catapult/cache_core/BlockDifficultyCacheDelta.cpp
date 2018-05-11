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

#include "BlockDifficultyCacheDelta.h"
#include "catapult/deltaset/BaseSetDeltaIterationView.h"
#include "catapult/exceptions.h"

namespace catapult { namespace cache {

	BasicBlockDifficultyCacheDelta::BasicBlockDifficultyCacheDelta(
			const BlockDifficultyCacheTypes::BaseSetDeltaPointers& difficultyInfoSets,
			const BlockDifficultyCacheTypes::Options& options)
			: BlockDifficultyCacheDeltaMixins::Size(*difficultyInfoSets.pPrimary)
			, BlockDifficultyCacheDeltaMixins::Contains(*difficultyInfoSets.pPrimary)
			, m_pOrderedDelta(difficultyInfoSets.pPrimary)
			, m_difficultyHistorySize(options.DifficultyHistorySize)
			// note: empty indicates initial cache seeding;
			//       it cannot happen due to a rollback because the nemesis block cannot be rolled back
			, m_startHeight(m_pOrderedDelta->empty() ? Height(1) : MakeIterableView(*m_pOrderedDelta).begin()->BlockHeight)
	{}

	deltaset::PruningBoundary<BasicBlockDifficultyCacheDelta::ValueType> BasicBlockDifficultyCacheDelta::pruningBoundary() const {
		return m_pruningBoundary;
	}

	void BasicBlockDifficultyCacheDelta::insert(const ValueType& info) {
		checkInsert(info.BlockHeight);
		m_pOrderedDelta->insert(info);
	}

	void BasicBlockDifficultyCacheDelta::insert(Height height, Timestamp timestamp, Difficulty difficulty) {
		checkInsert(height);
		m_pOrderedDelta->emplace(height, timestamp, difficulty);
	}

	void BasicBlockDifficultyCacheDelta::remove(const ValueType& info) {
		checkRemove(info.BlockHeight);
		remove(info.BlockHeight);
	}

	void BasicBlockDifficultyCacheDelta::remove(Height height) {
		checkRemove(height);
		m_pOrderedDelta->remove(ValueType(height));
	}

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
			CATAPULT_THROW_INVALID_ARGUMENT_2("insertion of element with unexpected height (nextHeight, height)", nextHeight(), height);
	}

	void BasicBlockDifficultyCacheDelta::checkRemove(Height height) const {
		// remove is only allowed if both
		// - the set is non-empty
		// - the height is equal to the largest height in the set
		if (m_pOrderedDelta->empty())
			CATAPULT_THROW_RUNTIME_ERROR("remove called on empty cache");

		if (nextHeight() != height + Height(1))
			CATAPULT_THROW_INVALID_ARGUMENT_2("removal of element with unexpected height (nextHeight, height)", nextHeight(), height);
	}

	Height BasicBlockDifficultyCacheDelta::nextHeight() const {
		return Height(m_startHeight.unwrap() + size());
	}
}}
