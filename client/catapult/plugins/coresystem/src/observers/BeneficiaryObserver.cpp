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
#include "src/importance/ActivityObserverUtils.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/cache_core/AccountStateCacheUtils.h"

namespace catapult { namespace observers {

	namespace {
		void UpdateBeneficiaryActivity(const Key& publicKey, ObserverContext& context) {
			auto& cache = context.Cache.template sub<cache::AccountStateCache>();
			cache::ProcessForwardedAccountState(cache, publicKey, [&context](auto& accountState) {
				importance::UpdateActivity(
					accountState.PublicKey,
					context,
					[](auto& bucket) { ++bucket.BeneficiaryCount; },
					[](auto& bucket) { --bucket.BeneficiaryCount; });
			});
		}

		bool IsOwnBlock(const Key& signer, const Key& harvesterBeneficiary) {
			return Key() == harvesterBeneficiary || signer == harvesterBeneficiary;
		}
	}

	DEFINE_OBSERVER(Beneficiary, model::BlockNotification, ([](const auto& notification, auto& context) {
		if (IsOwnBlock(notification.Signer, notification.Beneficiary))
			return;

		UpdateBeneficiaryActivity(notification.Beneficiary, context);
	}));
}}
