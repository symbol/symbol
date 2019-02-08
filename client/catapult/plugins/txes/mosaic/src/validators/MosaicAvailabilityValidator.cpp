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

	namespace {
		bool ContainsRequiredPropertyChange(const model::MosaicProperties& properties) {
			for (auto i = 0u; i < model::First_Optional_Property; ++i) {
				if (0 != (properties.begin() + i)->Value)
					return true;
			}

			return false;
		}
	}

	DEFINE_STATEFUL_VALIDATOR(MosaicAvailability, [](const auto& notification, const ValidatorContext& context) {
		const auto& cache = context.Cache.sub<cache::MosaicCache>();

		// always allow a new mosaic
		auto mosaicIter = cache.find(notification.MosaicId);
		if (!mosaicIter.tryGet())
			return ValidationResult::Success;

		// mosaic has to be active
		if (!cache.isActive(notification.MosaicId, context.Height))
			return Failure_Mosaic_Expired;

		// disallow a noop modification
		// 1) if any required property changed then it is not a noop modification
		// 2) if mosaic is eternal and duration property is non-zero, it is a change to non-eternal that the duration validator rejects,
		//    so only need to check current definition and reject if it is eternal
		const auto& entry = mosaicIter.get();
		auto requiredPropertyChanged = ContainsRequiredPropertyChange(notification.Properties);
		if (!requiredPropertyChanged && entry.definition().isEternal())
			return Failure_Mosaic_Modification_No_Changes;

		// require mosaic supply to be zero because else, when rolling back, the definition observer does not know
		// what the supply was before
		return Amount() != entry.supply() ? Failure_Mosaic_Modification_Disallowed : ValidationResult::Success;
	});
}}
