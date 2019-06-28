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

	namespace {
		bool ShouldPopBucket(const state::AccountActivityBuckets::ActivityBucket& bucket) {
			return model::ImportanceHeight() != bucket.StartHeight
					&& Amount() == bucket.TotalFeesPaid
					&& 0u == bucket.BeneficiaryCount
					&& 0u == bucket.RawScore;
		}
	}

	void UpdateActivity(
			const Key& publicKey,
			const observers::ObserverContext& context,
			const ActivityBucketConsumer& commitAction,
			const ActivityBucketConsumer& rollbackAction) {
		auto& accountStateCache = context.Cache.sub<cache::AccountStateCache>();
		auto accountStateIter = accountStateCache.find(publicKey);
		if (accountStateIter.get().Balances.get(accountStateCache.harvestingMosaicId()) < accountStateCache.minHarvesterBalance())
			return;

		auto& activityBuckets = accountStateIter.get().ActivityBuckets;
		auto importanceHeight = model::ConvertToImportanceHeight(context.Height, accountStateCache.importanceGrouping());
		if (observers::NotifyMode::Commit == context.Mode) {
			activityBuckets.update(importanceHeight, commitAction);
		} else {
			activityBuckets.tryUpdate(importanceHeight, rollbackAction);

			if (ShouldPopBucket(*activityBuckets.begin()))
				activityBuckets.pop();
		}
	}
}}
