#pragma once
#include "catapult/cache/CatapultCacheDelta.h"
#include "catapult/state/CatapultState.h"
#include <iosfwd>

namespace catapult { namespace observers {

/// Enumeration of possible notification modes.
#define NOTIFY_MODE_LIST \
	/* Execute actions. */ \
	ENUM_VALUE(Commit) \
	\
	/* Reverse actions. */ \
	ENUM_VALUE(Rollback)

#define DECLARE_ENUM NotifyMode
#define ENUM_LIST NOTIFY_MODE_LIST
#include "catapult/utils/MacroBasedEnum.h"
#undef ENUM_LIST
#undef DECLARE_ENUM

	/// Mutatable state passed to all the observers.
	struct ObserverState {
	public:
		/// Creates an observer state around \a cache and \a state.
		ObserverState(cache::CatapultCacheDelta& cache, state::CatapultState& state)
				: Cache(cache)
				, State(state)
		{}

	public:
		/// The catapult cache.
		cache::CatapultCacheDelta& Cache;

		/// The catapult state.
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
		constexpr ObserverContext(
				cache::CatapultCacheDelta& cache,
				state::CatapultState& state,
				Height height,
				NotifyMode mode)
				: Cache(cache)
				, State(state)
				, Height(height)
				, Mode(mode)
		{}

	public:
		/// The catapult cache.
		cache::CatapultCacheDelta& Cache;

		/// The catapult state.
		state::CatapultState& State;

		/// The current height.
		const catapult::Height Height;

		/// The notification mode.
		const NotifyMode Mode;
	};
}}
