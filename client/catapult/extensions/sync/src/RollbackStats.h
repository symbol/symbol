#pragma once
#include "catapult/types.h"
#include <list>

namespace catapult { namespace sync {

	/// Rollback counter types.
	enum class RollbackCounterType {
		/// Number of rollbacks since start of the server.
		All,

		/// Number of rollbacks within a time frame (configuration dependent).
		Recent,

		/// Longest rollback.
		Longest
	};

	/// Container for rollback statistics.
	class RollbackStats {
	public:
		/// Creates rollback statistics container.
		RollbackStats();

	public:
		/// Returns statistics for a type (\a rollbackCounterType).
		uint64_t total(RollbackCounterType rollbackCounterType) const;

		/// Adds info about \a rollbackSize at \a timestamp to current statistics.
		void add(Timestamp timestamp, size_t rollbackSize);

		/// Prunes statistics below time \a threshold.
		void prune(Timestamp threshold);

	private:
		struct RollbackStatsEntry {
			catapult::Timestamp Timestamp;
			size_t RollbackSize;
		};

	private:
		uint64_t m_totalRollbacks;
		std::list<RollbackStatsEntry> m_rollbackSizes;
		uint64_t m_longestRollback;
	};
}}
