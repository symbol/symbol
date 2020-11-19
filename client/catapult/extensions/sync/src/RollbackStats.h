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
		/// Gets the statistics value for the specified counter type (\a rollbackCounterType).
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
