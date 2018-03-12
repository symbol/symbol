#pragma once
#include "RollbackStats.h"
#include "catapult/chain/ChainFunctions.h"
#include "catapult/utils/TimeSpan.h"
#include "catapult/types.h"
#include <vector>

namespace catapult { namespace sync {

	/// Rollback results.
	enum class RollbackResult {
		/// Committed rollback counters.
		Committed,

		/// Ignored rollback counters.
		Ignored
	};

	/// Container for both committed and ignored rollback statistics.
	class RollbackInfo {
	public:
		/// Creates a container around \a timeSupplier with recent stats span (\a recentStatsTimeSpan).
		explicit RollbackInfo(const chain::TimeSupplier& timeSupplier, const utils::TimeSpan& recentStatsTimeSpan);

	public:
		/// Returns rollback counter for result (\a rollbackResult) and counter type (\a rollbackCounterType).
		size_t counter(RollbackResult rollbackResult, RollbackCounterType rollbackCounterType) const;

	public:
		/// Increments counter.
		void increment();

		/// Adds counter to ignored statistics and resets the counter.
		void reset();

		/// Adds counter to committed statistics and resets the counter.
		void save();

	private:
		void prune();

	private:
		chain::TimeSupplier m_timeSupplier;
		const utils::TimeSpan m_recentStatsTimeSpan;
		size_t m_currentRollbackSize;

		RollbackStats m_committed;
		RollbackStats m_ignored;
	};
}}
