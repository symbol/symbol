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
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::MosaicDefinitionNotification;

	DECLARE_STATEFUL_VALIDATOR(MosaicDuration, Notification)(BlockDuration maxMosaicDuration) {
		return MAKE_STATEFUL_VALIDATOR(MosaicDuration, [maxMosaicDuration](const auto& notification, const ValidatorContext& context) {
			const auto& cache = context.Cache.sub<cache::MosaicCache>();

			// always allow a new mosaic (MosaicPropertiesValidator checks for valid duration in this case)
			auto mosaicIter = cache.find(notification.MosaicId);
			if (!mosaicIter.tryGet())
				return ValidationResult::Success;

			const auto& entry = mosaicIter.get();
			auto currentDuration = entry.definition().properties().duration();
			auto delta = notification.Properties.duration();
			auto resultingDuration = currentDuration + delta;
			auto isIncompatibleChange =
					(BlockDuration() == currentDuration && BlockDuration() != delta) ||
					(BlockDuration() != currentDuration && BlockDuration() == delta);
			return isIncompatibleChange || maxMosaicDuration < resultingDuration || resultingDuration < currentDuration
					? Failure_Mosaic_Invalid_Duration
					: ValidationResult::Success;
		});
	}
}}
