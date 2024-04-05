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

#include "Observers.h"
#include "src/cache/SecretLockInfoCache.h"
#include "src/model/SecretLockReceiptType.h"
#include "plugins/txes/lock_shared/src/observers/ExpiredLockInfoObserver.h"
#include "catapult/observers/ObserverUtils.h"

namespace catapult { namespace observers {

	DECLARE_OBSERVER(ExpiredSecretLockInfo, model::BlockNotification)(
			const std::unordered_set<Height, utils::BaseValueHasher<Height>>& skipHeights,
			const std::unordered_set<Height, utils::BaseValueHasher<Height>>& forceHeights) {
		return MAKE_OBSERVER(ExpiredSecretLockInfo, model::BlockNotification, ([skipHeights, forceHeights](
				const auto&,
				ObserverContext& context) {
			if (skipHeights.cend() != skipHeights.find(context.Height)) // bypass expiration for specified heights
				return;

			const auto Expired_Receipt_Type = model::Receipt_Type_LockSecret_Expired;
			ExpiredLockInfoObserver<cache::SecretLockInfoCache>(context, Expired_Receipt_Type, [height = context.Height, &forceHeights](
					auto& accountStateCache,
					const auto& lockInfo,
					auto accountStateConsumer) {
				// only process if last lock in history is expiring or processing is forced
				if (height == lockInfo.EndHeight || forceHeights.cend() != forceHeights.find(height))
					accountStateConsumer(accountStateCache.find(lockInfo.OwnerAddress).get());
			});
		}));
	}
}}
