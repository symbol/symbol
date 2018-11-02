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

	using Notification = model::MosaicDefinitionNotification;

	DEFINE_STATEFUL_VALIDATOR(MosaicAvailability, [](const auto& notification, const ValidatorContext& context) {
		const auto& cache = context.Cache.sub<cache::MosaicCache>();

		// - always allow a new mosaic
		if (!cache.isActive(notification.MosaicId, context.Height))
			return ValidationResult::Success;

		// - disallow conflicting parent namespace
		auto mosaicIter = cache.find(notification.MosaicId);
		const auto& entry = mosaicIter.get();
		if (entry.namespaceId() != notification.ParentId)
			return Failure_Mosaic_Parent_Id_Conflict;

		// - disallow a noop modification
		if (notification.Properties == entry.definition().properties())
			return Failure_Mosaic_Modification_No_Changes;

		// - only allow a modification if signer contains full balance
		const auto& accountStateCache = context.Cache.sub<cache::AccountStateCache>();
		auto accountStateIter = accountStateCache.find(notification.Signer);
		return !accountStateIter.tryGet() || entry.supply() != accountStateIter.get().Balances.get(notification.MosaicId)
				? Failure_Mosaic_Modification_Disallowed
				: ValidationResult::Success;
	});
}}
