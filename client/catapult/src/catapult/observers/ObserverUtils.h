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

	/// Returns \c true if \a action and \a notifyMode indicate that a link should be made.
	template<typename TAction>
	constexpr bool ShouldLink(TAction action, NotifyMode notifyMode) {
		return NotifyMode::Commit == notifyMode ? TAction::Link == action : TAction::Unlink == action;
	}

	/// Creates a time-based cache pruning observer with \a name that runs every \a interval blocks.
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

	/// Creates a block-based cache touch observer with \a name that touches the cache at every block height taking into account
	/// \a gracePeriod and creates a receipt of type \a receiptType for all deactivating elements.
	template<typename TCache>
	NotificationObserverPointerT<model::BlockNotification> CreateCacheBlockTouchObserver(
			const std::string& name,
			model::ReceiptType receiptType,
			BlockDuration gracePeriod = BlockDuration()) {
		using ObserverType = FunctionalNotificationObserverT<model::BlockNotification>;
		return std::make_unique<ObserverType>(name + "TouchObserver", [receiptType, gracePeriod](const auto&, auto& context) {
			auto touchHeight = Height(context.Height.unwrap() + gracePeriod.unwrap());
			auto& cache = context.Cache.template sub<TCache>();
			auto expiryIds = cache.touch(touchHeight);

			if (NotifyMode::Rollback == context.Mode)
				return;

			// sort expiry ids because receipts must be generated deterministically
			std::set<typename decltype(expiryIds)::value_type> orderedExpiryIds(expiryIds.cbegin(), expiryIds.cend());
			for (auto id : orderedExpiryIds)
				context.StatementBuilder().addReceipt(model::ArtifactExpiryReceipt<decltype(id)>(receiptType, id));
		});
	}
}}
