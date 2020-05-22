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

#include "ActivityObserverUtils.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/observers/ObserverContext.h"

namespace catapult { namespace importance {

	void UpdateActivity(
			const Address& address,
			const observers::ObserverContext& context,
			const ActivityBucketConsumer& commitAction,
			const ActivityBucketConsumer& rollbackAction) {
		auto& accountStateCache = context.Cache.sub<cache::AccountStateCache>();
		auto accountStateIter = accountStateCache.find(address);

		auto& activityBuckets = accountStateIter.get().ActivityBuckets;
		auto importanceHeight = model::ConvertToImportanceHeight(context.Height, accountStateCache.importanceGrouping());

		activityBuckets.tryUpdate(importanceHeight, observers::NotifyMode::Commit == context.Mode ? commitAction : rollbackAction);
	}
}}
