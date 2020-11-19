/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "BlockStatisticCacheDelta.h"
#include "catapult/exceptions.h"

namespace catapult { namespace cache {

	BasicBlockStatisticCacheDelta::BasicBlockStatisticCacheDelta(
			const BlockStatisticCacheTypes::BaseSetDeltaPointers& statisticSets,
			const BlockStatisticCacheTypes::Options& options)
			: BlockStatisticCacheDeltaMixins::Size(*statisticSets.pPrimary)
			, BlockStatisticCacheDeltaMixins::Contains(*statisticSets.pPrimary)
			, BlockStatisticCacheDeltaMixins::DeltaElements(*statisticSets.pPrimary)
			, BlockStatisticCacheDeltaMixins::BlockStatisticRange(*statisticSets.pPrimary)
			, m_pOrderedDelta(statisticSets.pPrimary)
			, m_historySize(options.HistorySize)
			// note: empty indicates initial cache seeding;
			//       it cannot happen due to a rollback because the nemesis block cannot be rolled back
			, m_startHeight(m_pOrderedDelta->empty() ? Height(1) : MakeIterableView(*m_pOrderedDelta).begin()->Height)
	{}

	deltaset::PruningBoundary<BasicBlockStatisticCacheDelta::ValueType> BasicBlockStatisticCacheDelta::pruningBoundary() const {
		return m_pruningBoundary;
	}

	std::unique_ptr<BasicBlockStatisticCacheDelta::IterableView> BasicBlockStatisticCacheDelta::tryMakeIterableView() const {
		return std::make_unique<IterableView>(*m_pOrderedDelta);
	}

	void BasicBlockStatisticCacheDelta::insert(const ValueType& statistic) {
		checkInsert(statistic.Height);
		m_pOrderedDelta->insert(statistic);
	}

	void BasicBlockStatisticCacheDelta::remove(const ValueType& statistic) {
		checkRemove(statistic.Height);
		remove(statistic.Height);
	}

	void BasicBlockStatisticCacheDelta::remove(Height height) {
		checkRemove(height);
		m_pOrderedDelta->remove(ValueType(height));
	}

	void BasicBlockStatisticCacheDelta::prune(Height height) {
		auto heightDifference = Height(m_historySize);
		if (height > heightDifference)
			m_pruningBoundary = deltaset::PruningBoundary<ValueType>(ValueType(height - heightDifference));
	}

	namespace {
		[[noreturn]]
		void ThrowInvalidHeightError(const char* operation, Height nextCacheHeight, Height elementHeight) {
			std::ostringstream out;
			out
					<< "cannot " << operation << " element with height " << elementHeight
					<< " when cache height is " << (nextCacheHeight - Height(1));
			CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
		}
	}

	void BasicBlockStatisticCacheDelta::checkInsert(Height height) {
		// insert is only allowed if either
		// - the set is empty
		// - the height is one larger than the largest height in the set
		if (m_pOrderedDelta->empty()) {
			m_startHeight = height;
			return;
		}

		if (nextHeight() != height)
			ThrowInvalidHeightError("insert", nextHeight(), height);
	}

	void BasicBlockStatisticCacheDelta::checkRemove(Height height) const {
		// remove is only allowed if both
		// - the set is not empty
		// - the height is equal to the largest height in the set
		if (m_pOrderedDelta->empty())
			CATAPULT_THROW_RUNTIME_ERROR("remove called on empty cache");

		if (nextHeight() != height + Height(1))
			ThrowInvalidHeightError("remove", nextHeight(), height);
	}

	Height BasicBlockStatisticCacheDelta::nextHeight() const {
		return Height(m_startHeight.unwrap() + size());
	}
}}
