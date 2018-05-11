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
#include "catapult/cache/CatapultCacheDelta.h"
#include "catapult/state/CatapultState.h"
#include <iosfwd>

namespace catapult { namespace observers {

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

	/// Mutatable state passed to all the observers.
	struct ObserverState {
	public:
		/// Creates an observer state around \a cache and \a state.
		ObserverState(cache::CatapultCacheDelta& cache, state::CatapultState& state)
				: Cache(cache)
				, State(state)
		{}

	public:
		/// Catapult cache.
		cache::CatapultCacheDelta& Cache;

		/// Catapult state.
		state::CatapultState& State;
	};

	/// Context passed to all the observers.
	struct ObserverContext {
	public:
		/// Creates an observer context around \a state at \a height with specified \a mode.
		constexpr ObserverContext(const ObserverState& state, Height height, NotifyMode mode)
				: ObserverContext(state.Cache, state.State, height, mode)
		{}

		/// Creates an observer context around \a cache and \a state at \a height with specified \a mode.
		constexpr ObserverContext(cache::CatapultCacheDelta& cache, state::CatapultState& state, Height height, NotifyMode mode)
				: Cache(cache)
				, State(state)
				, Height(height)
				, Mode(mode)
		{}

	public:
		/// Catapult cache.
		cache::CatapultCacheDelta& Cache;

		/// Catapult state.
		state::CatapultState& State;

		/// Current height.
		const catapult::Height Height;

		/// Notification mode.
		const NotifyMode Mode;
	};
}}
