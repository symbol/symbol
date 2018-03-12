#include "RollbackStats.h"

namespace catapult { namespace sync {

	RollbackStats::RollbackStats()
			: m_totalRollbacks(0)
			, m_longestRollback(0)
	{}

	uint64_t RollbackStats::total(RollbackCounterType rollbackCounterType) const {
		switch (rollbackCounterType) {
			case RollbackCounterType::All:
				return m_totalRollbacks;

			case RollbackCounterType::Recent:
				return m_rollbackSizes.size();

			case RollbackCounterType::Longest:
				return m_longestRollback;
		}

		return 0;
	}

	void RollbackStats::add(Timestamp timestamp, size_t rollbackSize) {
		if (0 == rollbackSize)
			return;

		m_rollbackSizes.push_back({ timestamp, rollbackSize });
		m_longestRollback = std::max<uint64_t>(m_longestRollback, rollbackSize);
		++m_totalRollbacks;
	}

	void RollbackStats::prune(Timestamp threshold) {
		auto iter = m_rollbackSizes.begin();
		while (iter != m_rollbackSizes.end()) {
			if (iter->Timestamp > threshold)
				break;

			iter = m_rollbackSizes.erase(iter);
		}
	}
}}
