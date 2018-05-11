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
#include "LockInfoObserverUtils.h"
#include "src/cache/HashLockInfoCache.h"
#include "plugins/txes/aggregate/src/model/AggregateEntityType.h"

namespace catapult { namespace observers {

	using Notification = model::TransactionNotification;

	namespace {
		struct HashTraits {
		public:
			using CacheType = cache::HashLockInfoCache;
			using Notification = observers::Notification;

			static auto NotificationToKey(const Notification& notification) {
				return notification.TransactionHash;
			}

			static auto DestinationAccount(const model::HashLockInfo& lockInfo) {
				return lockInfo.Account;
			}
		};
	}

	DEFINE_OBSERVER(CompletedAggregate, Notification, [](const auto& notification, const auto& context) {
		if (model::Entity_Type_Aggregate_Bonded != notification.TransactionType)
			return;

		LockStatusAccountBalanceObserver<HashTraits>(notification, context);
	});
}}
