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
#include "RollbackStats.h"
#include "catapult/chain/ChainFunctions.h"
#include "catapult/utils/SpinReaderWriterLock.h"
#include "catapult/utils/TimeSpan.h"
#include "catapult/types.h"
#include <vector>

namespace catapult { namespace sync { struct RollbackInfoState; } }

namespace catapult { namespace sync {

	/// Rollback results.
	enum class RollbackResult {
		/// Committed rollback counters.
		Committed,

		/// Ignored rollback counters.
		Ignored
	};

	/// Read only view on top of rollback info.
	class RollbackInfoView : utils::MoveOnly {
	public:
		/// Creates a view around \a committed and \a ignored with lock context \a readLock.
		RollbackInfoView(
				const RollbackStats& committed,
				const RollbackStats& ignored,
				utils::SpinReaderWriterLock::ReaderLockGuard&& readLock);

	public:
		/// Gets the rollback counter for result (\a rollbackResult) and counter type (\a rollbackCounterType).
		size_t counter(RollbackResult rollbackResult, RollbackCounterType rollbackCounterType) const;

	private:
		const RollbackStats& m_committed;
		const RollbackStats& m_ignored;
		utils::SpinReaderWriterLock::ReaderLockGuard m_readLock;
	};

	/// Write only view on top of rollback info.
	class RollbackInfoModifier : utils::MoveOnly {
	public:
		/// Creates a view around \a committed, \a ignored and \a state with lock context \a writeLock.
		RollbackInfoModifier(
				RollbackStats& committed,
				RollbackStats& ignored,
				RollbackInfoState& state,
				utils::SpinReaderWriterLock::WriterLockGuard&& writeLock);

	public:
		/// Increments counter.
		void increment();

		/// Adds counter to ignored statistics and resets the counter.
		void reset();

		/// Adds counter to committed statistics and resets the counter.
		void save();

	private:
		void add(RollbackStats& target);

		void prune();

	private:
		RollbackStats& m_committed;
		RollbackStats& m_ignored;
		RollbackInfoState& m_state;
		utils::SpinReaderWriterLock::WriterLockGuard m_writeLock;
	};

	/// Container for both committed and ignored rollback statistics.
	class RollbackInfo {
	public:
		/// Creates a container around \a timeSupplier with recent stats span (\a recentStatsTimeSpan).
		RollbackInfo(const chain::TimeSupplier& timeSupplier, const utils::TimeSpan& recentStatsTimeSpan);

		/// Destroys the container.
		~RollbackInfo();

	public:
		/// Gets a read only view of the rollback info.
		RollbackInfoView view() const;

		/// Gets a write only view of the rollback info.
		RollbackInfoModifier modifier();

	private:
		std::unique_ptr<RollbackInfoState> m_pState;

		RollbackStats m_committed;
		RollbackStats m_ignored;
		mutable utils::SpinReaderWriterLock m_lock;
	};

}}
