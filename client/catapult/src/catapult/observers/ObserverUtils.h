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
