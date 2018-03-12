#include "RollbackInfo.h"
#include "catapult/model/Elements.h"

namespace catapult { namespace sync {

	RollbackInfo::RollbackInfo(const chain::TimeSupplier& timeSupplier, const utils::TimeSpan& recentStatsTimeSpan)
			: m_timeSupplier(timeSupplier)
			, m_recentStatsTimeSpan(recentStatsTimeSpan)
			, m_currentRollbackSize(0)
	{}

	size_t RollbackInfo::counter(RollbackResult rollbackResult, RollbackCounterType rollbackCounterType) const {
		return RollbackResult::Committed == rollbackResult
				? m_committed.total(rollbackCounterType)
				: m_ignored.total(rollbackCounterType);
	}

	void RollbackInfo::increment() {
		++m_currentRollbackSize;
	}

	void RollbackInfo::reset() {
		m_ignored.add(m_timeSupplier(), m_currentRollbackSize);
		m_currentRollbackSize = 0;

		prune();
	}

	void RollbackInfo::save() {
		m_committed.add(m_timeSupplier(), m_currentRollbackSize);
		m_currentRollbackSize = 0;

		prune();
	}

	void RollbackInfo::prune() {
		auto threshold = utils::SubtractNonNegative(m_timeSupplier(), m_recentStatsTimeSpan);

		m_committed.prune(threshold);
		m_ignored.prune(threshold);
	}
}}
