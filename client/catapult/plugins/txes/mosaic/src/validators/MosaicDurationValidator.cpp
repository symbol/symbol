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

#include "Validators.h"
#include "src/cache/MosaicCache.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::MosaicDefinitionNotification;

	DECLARE_STATEFUL_VALIDATOR(MosaicDuration, Notification)(BlockDuration maxMosaicDuration) {
		return MAKE_STATEFUL_VALIDATOR(MosaicDuration, [maxMosaicDuration](
				const Notification& notification,
				const ValidatorContext& context) {
			// skip this validator is there is no duration change
			if (BlockDuration() == notification.Properties.duration())
				return ValidationResult::Success;

			// as an optimization, since duration is additive, check notification before checking cache
			if (maxMosaicDuration < notification.Properties.duration())
				return Failure_Mosaic_Invalid_Duration;

			// allow a new mosaic because checks above passed
			const auto& cache = context.Cache.sub<cache::MosaicCache>();
			auto mosaicIter = cache.find(notification.MosaicId);
			if (!mosaicIter.tryGet())
				return ValidationResult::Success;

			const auto& mosaicEntry = mosaicIter.get();
			auto currentDuration = mosaicEntry.definition().properties().duration();

			// cannot change eternal durations
			if (BlockDuration() == currentDuration)
				return Failure_Mosaic_Invalid_Duration;

			auto resultingDuration = currentDuration + notification.Properties.duration();
			return maxMosaicDuration < resultingDuration || resultingDuration < currentDuration
					? Failure_Mosaic_Invalid_Duration
					: ValidationResult::Success;
		});
	}
}}
