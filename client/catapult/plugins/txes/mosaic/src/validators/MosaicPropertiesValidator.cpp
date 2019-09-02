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
#include "catapult/constants.h"

namespace catapult { namespace validators {

	using Notification = model::MosaicPropertiesNotification;

	namespace {
		constexpr bool IsValidFlags(model::MosaicFlags flags) {
			return flags <= model::MosaicFlags::All;
		}
	}

	DECLARE_STATELESS_VALIDATOR(MosaicProperties, Notification)(uint8_t maxDivisibility, BlockDuration maxMosaicDuration) {
		return MAKE_STATELESS_VALIDATOR(MosaicProperties, ([maxDivisibility, maxMosaicDuration](const Notification& notification) {
			if (!IsValidFlags(notification.Properties.flags()))
				return Failure_Mosaic_Invalid_Flags;

			if (notification.Properties.divisibility() > maxDivisibility)
				return Failure_Mosaic_Invalid_Divisibility;

			return maxMosaicDuration < notification.Properties.duration()
					? Failure_Mosaic_Invalid_Duration
					: ValidationResult::Success;
		}));
	}
}}
