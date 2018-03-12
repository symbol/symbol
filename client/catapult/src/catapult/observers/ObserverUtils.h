#pragma once
#include "ObserverContext.h"
#include "ObserverTypes.h"

namespace catapult { namespace observers {

	/// Returns \c true if \a context and \a pruneInterval indicate that pruning should be done.
	constexpr bool ShouldPrune(const ObserverContext& context, size_t pruneInterval) {
		return NotifyMode::Commit == context.Mode && 0 == context.Height.unwrap() % pruneInterval;
	}

	/// Creates a block-based cache pruning obsever with \a name that runs every \a interval blocks
	/// with the specified grace period (\a gracePeriod).
	template<typename TCache>
	NotificationObserverPointerT<model::BlockNotification> CreateCacheBlockPruningObserver(
			const std::string& name,
			size_t interval,
			BlockDuration gracePeriod) {
		using ObserverType = FunctionalNotificationObserverT<model::BlockNotification>;
		return std::make_unique<ObserverType>(name + "PruningObserver", [gracePeriod, interval](const auto&, const auto& context) {
			if (!ShouldPrune(context, interval))
				return;

			if (context.Height.unwrap() <= gracePeriod.unwrap())
				return;

			auto pruneHeight = Height(context.Height.unwrap() - gracePeriod.unwrap());
			auto& cache = context.Cache.template sub<TCache>();
			cache.prune(pruneHeight);
		});
	}

	/// Creates a time-based cache pruning obsever with \a name that runs every \a interval blocks.
	template<typename TCache>
	NotificationObserverPointerT<model::BlockNotification> CreateCacheTimePruningObserver(const std::string& name, size_t interval) {
		using ObserverType = FunctionalNotificationObserverT<model::BlockNotification>;
		return std::make_unique<ObserverType>(name + "PruningObserver", [interval](const auto& notification, const auto& context) {
			if (!ShouldPrune(context, interval))
				return;

			auto& cache = context.Cache.template sub<TCache>();
			cache.prune(notification.Timestamp);
		});
	}
}}
