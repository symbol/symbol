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

	DECLARE_STATEFUL_VALIDATOR(MosaicDivisibility, Notification)(uint8_t maxDivisibility) {
		return MAKE_STATEFUL_VALIDATOR(MosaicDivisibility, [maxDivisibility](
				const Notification& notification,
				const ValidatorContext& context) {
			auto newDivisibility = notification.Properties.divisibility();

			const auto& cache = context.Cache.sub<cache::MosaicCache>();
			auto mosaicIter = cache.find(notification.MosaicId);
			if (mosaicIter.tryGet())
				newDivisibility = static_cast<uint8_t>(newDivisibility ^ mosaicIter.get().definition().properties().divisibility());

			return newDivisibility > maxDivisibility ? Failure_Mosaic_Invalid_Divisibility : ValidationResult::Success;
		});
	}
}}
