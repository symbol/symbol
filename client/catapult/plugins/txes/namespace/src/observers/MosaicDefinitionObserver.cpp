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

	namespace {
		void ZeroBalance(state::AccountState& accountState, MosaicId mosaicId) {
			auto ownerBalance = accountState.Balances.get(mosaicId);
			accountState.Balances.debit(mosaicId, ownerBalance);
		}
	}

	DEFINE_OBSERVER(MosaicDefinition, model::MosaicDefinitionNotification, [](const auto& notification, const ObserverContext& context) {
		auto& accountStateCache = context.Cache.sub<cache::AccountStateCache>();
		auto& cache = context.Cache.sub<cache::MosaicCache>();

		// always zero the owner's balance when a mosaic definition changes (in case of rollback, it will be fixed below)
		auto& ownerState = accountStateCache.get(notification.Signer);
		ZeroBalance(ownerState, notification.MosaicId);

		if (NotifyMode::Rollback == context.Mode) {
			cache.remove(notification.MosaicId);

			// mosaic is not completely removed from the cache if it initially had a history depth greater than one
			if (cache.contains(notification.MosaicId)) {
				// set the owner's balance to the full supply
				const auto& mosaicEntry = cache.get(notification.MosaicId);
				ownerState.Balances.credit(notification.MosaicId, mosaicEntry.supply());
			}

			return;
		}

		auto definition = state::MosaicDefinition(context.Height, notification.Signer, notification.Properties);
		cache.insert(state::MosaicEntry(notification.ParentId, notification.MosaicId, definition));
	});
}}
