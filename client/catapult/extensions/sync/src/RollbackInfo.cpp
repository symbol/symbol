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

#include "RollbackInfo.h"
#include "catapult/model/Elements.h"

namespace catapult { namespace sync {

	// region RollbackInfoState

	struct RollbackInfoState {
	public:
		RollbackInfoState(const chain::TimeSupplier& timeSupplier, const utils::TimeSpan& recentStatsTimeSpan)
				: TimeSupplier(timeSupplier)
				, RecentStatsTimeSpan(recentStatsTimeSpan)
				, CurrentRollbackSize(0)
		{}

	public:
		chain::TimeSupplier TimeSupplier;
		const utils::TimeSpan RecentStatsTimeSpan;
		size_t CurrentRollbackSize;
	};

	// endregion

	// region RollbackInfoView

	RollbackInfoView::RollbackInfoView(
			const RollbackStats& committed,
			const RollbackStats& ignored,
			utils::SpinReaderWriterLock::ReaderLockGuard&& readLock)
			: m_committed(committed)
			, m_ignored(ignored)
			, m_readLock(std::move(readLock))
	{}

	size_t RollbackInfoView::counter(RollbackResult rollbackResult, RollbackCounterType rollbackCounterType) const {
		return RollbackResult::Committed == rollbackResult
				? m_committed.total(rollbackCounterType)
				: m_ignored.total(rollbackCounterType);
	}

	// endregion

	// region RollbackInfoModifier

	RollbackInfoModifier::RollbackInfoModifier(
			RollbackStats& committed,
			RollbackStats& ignored,
			RollbackInfoState& state,
			utils::SpinReaderWriterLock::WriterLockGuard&& writeLock)
			: m_committed(committed)
			, m_ignored(ignored)
			, m_state(state)
			, m_writeLock(std::move(writeLock))
	{}

	void RollbackInfoModifier::increment() {
		++m_state.CurrentRollbackSize;
	}

	void RollbackInfoModifier::reset() {
		add(m_ignored);
	}

	void RollbackInfoModifier::save() {
		add(m_committed);
	}

	void RollbackInfoModifier::add(RollbackStats& target) {
		target.add(m_state.TimeSupplier(), m_state.CurrentRollbackSize);
		m_state.CurrentRollbackSize = 0;

		prune();
	}

	void RollbackInfoModifier::prune() {
		auto threshold = utils::SubtractNonNegative(m_state.TimeSupplier(), m_state.RecentStatsTimeSpan);

		m_committed.prune(threshold);
		m_ignored.prune(threshold);
	}

	// endregion

	// region RollbackInfo

	RollbackInfo::RollbackInfo(const chain::TimeSupplier& timeSupplier, const utils::TimeSpan& recentStatsTimeSpan)
			: m_pState(std::make_unique<RollbackInfoState>(timeSupplier, recentStatsTimeSpan))
	{}

	RollbackInfo::~RollbackInfo() = default;

	RollbackInfoView RollbackInfo::view() const {
		auto readLock = m_lock.acquireReader();
		return RollbackInfoView(m_committed, m_ignored, std::move(readLock));
	}

	RollbackInfoModifier RollbackInfo::modifier() {
		auto writeLock = m_lock.acquireWriter();
		return RollbackInfoModifier(m_committed, m_ignored, *m_pState, std::move(writeLock));
	}

	// endregion
}}
