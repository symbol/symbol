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
#include "src/cache/HashLockInfoCache.h"
#include "src/model/LockInfo.h"
#include "catapult/cache_core/AccountStateCache.h"

namespace catapult { namespace observers {

	using Notification = model::HashLockNotification;

	namespace {
		auto CreateLockInfo(const Key& account, Height endHeight, const Notification& notification) {
			return model::HashLockInfo(account, notification.Mosaic.MosaicId, notification.Mosaic.Amount, endHeight, notification.Hash);
		}
	}

	DEFINE_OBSERVER(HashLock, Notification, [](const auto& notification, const ObserverContext& context) {
		auto& accountStateCache = context.Cache.sub<cache::AccountStateCache>();
		auto& signer = accountStateCache.get(notification.Signer);
		auto& cache = context.Cache.sub<cache::HashLockInfoCache>();

		if (NotifyMode::Commit == context.Mode) {
			auto endHeight = context.Height + Height(notification.Duration.unwrap());
			cache.insert(CreateLockInfo(signer.PublicKey, endHeight, notification));
			signer.Balances.debit(notification.Mosaic.MosaicId, notification.Mosaic.Amount);
		} else {
			cache.remove(notification.Hash);
			signer.Balances.credit(notification.Mosaic.MosaicId, notification.Mosaic.Amount);
		}
	});
}}
