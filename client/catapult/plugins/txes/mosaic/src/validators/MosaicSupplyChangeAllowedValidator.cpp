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

#include "Validators.h"
#include "src/cache/MosaicCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::MosaicSupplyChangeNotification;

	DECLARE_STATEFUL_VALIDATOR(MosaicSupplyChangeAllowed, Notification)(Amount maxAtomicUnits) {
		return MAKE_STATEFUL_VALIDATOR(MosaicSupplyChangeAllowed, [maxAtomicUnits](
				const Notification& notification,
				const ValidatorContext& context) {
			// notice that RequiredMosaicValidator will run first, so both mosaic and owning account must exist
			auto mosaicId = context.Resolvers.resolve(notification.MosaicId);
			const auto& cache = context.Cache.sub<cache::MosaicCache>();
			auto mosaicIter = cache.find(mosaicId);
			const auto& mosaicEntry = mosaicIter.get();

			const auto& accountStateCache = context.Cache.sub<cache::AccountStateCache>();
			auto accountStateIter = accountStateCache.find(notification.Owner);
			auto ownerAmount = accountStateIter.get().Balances.get(mosaicId);

			// only allow an "immutable" supply to change if the owner owns full supply
			const auto& properties = mosaicEntry.definition().properties();
			if (!properties.is(model::MosaicFlags::Supply_Mutable) && ownerAmount != mosaicEntry.supply())
				return Failure_Mosaic_Supply_Immutable;

			if (model::MosaicSupplyChangeAction::Decrease == notification.Action)
				return ownerAmount < notification.Delta ? Failure_Mosaic_Supply_Negative : ValidationResult::Success;

			// check that new supply does not overflow and is not too large
			auto initialSupply = mosaicEntry.supply();
			auto newSupply = mosaicEntry.supply() + notification.Delta;
			return newSupply < initialSupply || newSupply > maxAtomicUnits
					? Failure_Mosaic_Supply_Exceeded
					: ValidationResult::Success;
		});
	}
}}
