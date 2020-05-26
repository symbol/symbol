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

#pragma once
#include "ObserverStatementBuilder.h"
#include "catapult/cache/CatapultCacheDelta.h"
#include "catapult/model/NotificationContext.h"
#include "catapult/state/CatapultState.h"
#include <iosfwd>

namespace catapult { namespace observers {

	// region NotifyMode

#define NOTIFY_MODE_LIST \
	/* Execute actions. */ \
	ENUM_VALUE(Commit) \
	\
	/* Reverse actions. */ \
	ENUM_VALUE(Rollback)

#define ENUM_VALUE(LABEL) LABEL,
	/// Enumeration of possible notification modes.
	enum class NotifyMode {
		NOTIFY_MODE_LIST
	};
#undef ENUM_VALUE

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, NotifyMode value);

	// endregion

	// region ObserverState

	/// Block independent mutable state passed to all observers.
	struct ObserverState {
	public:
		/// Creates an observer state around \a cache.
		explicit ObserverState(cache::CatapultCacheDelta& cache);

		/// Creates an observer state around \a cache and \a blockStatementBuilder.
		ObserverState(cache::CatapultCacheDelta& cache, model::BlockStatementBuilder& blockStatementBuilder);

	public:
		/// Catapult cache.
		cache::CatapultCacheDelta& Cache;

		/// Optional block statement builder.
		model::BlockStatementBuilder* pBlockStatementBuilder;
	};

	// endregion

	// region ObserverContext

	/// Context passed to all the observers.
	struct ObserverContext : public model::NotificationContext {
	public:
		/// Creates an observer context around \a notificationContext, \a state and \a mode.
		/// \note \a state is const to enable more consise code even though it only contains non-const references.
		ObserverContext(const model::NotificationContext& notificationContext, const ObserverState& state, NotifyMode mode);

	public:
		/// Catapult cache.
		cache::CatapultCacheDelta& Cache;

		/// Notification mode.
		const NotifyMode Mode;

	public:
		/// Statement builder.
		ObserverStatementBuilder& StatementBuilder();

	private:
		ObserverStatementBuilder m_statementBuilder;
	};

	// endregion
}}
