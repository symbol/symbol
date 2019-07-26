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

#include "Observers.h"
#include "src/cache/MosaicCache.h"
#include "catapult/cache_core/AccountStateCache.h"

namespace catapult { namespace observers {

	using Notification = model::MosaicSupplyChangeNotification;

	namespace {
		constexpr bool ShouldIncrease(NotifyMode mode, model::MosaicSupplyChangeDirection direction) {
			return
					(NotifyMode::Commit == mode && model::MosaicSupplyChangeDirection::Increase == direction) ||
					(NotifyMode::Rollback == mode && model::MosaicSupplyChangeDirection::Decrease == direction);
		}
	}

	DEFINE_OBSERVER(MosaicSupplyChange, Notification, [](const Notification& notification, const ObserverContext& context) {
		auto mosaicId = context.Resolvers.resolve(notification.MosaicId);
		auto& accountStateCache = context.Cache.sub<cache::AccountStateCache>();
		auto& cache = context.Cache.sub<cache::MosaicCache>();

		auto accountStateIter = accountStateCache.find(notification.Signer);
		auto& accountState = accountStateIter.get();

		auto mosaicIter = cache.find(mosaicId);
		auto& entry = mosaicIter.get();
		if (ShouldIncrease(context.Mode, notification.Direction)) {
			accountState.Balances.credit(mosaicId, notification.Delta);
			entry.increaseSupply(notification.Delta);
		} else {
			accountState.Balances.debit(mosaicId, notification.Delta);
			entry.decreaseSupply(notification.Delta);
		}
	});
}}
