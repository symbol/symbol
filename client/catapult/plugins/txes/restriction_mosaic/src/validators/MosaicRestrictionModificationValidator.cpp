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
#include "src/cache/MosaicRestrictionCache.h"
#include "src/state/MosaicAddressRestrictionNotificationFacade.h"
#include "src/state/MosaicGlobalRestrictionNotificationFacade.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	namespace {
		using GlobalNotificationFacade = state::MosaicGlobalRestrictionNotificationFacade<
			model::MosaicGlobalRestrictionModificationPreviousValueNotification::Notification_Type>;
		using AddressNotificationFacade = state::MosaicAddressRestrictionNotificationFacade<
			model::MosaicAddressRestrictionModificationPreviousValueNotification::Notification_Type>;

		template<typename TNotificationFacade>
		ValidationResult MosaicRestrictionModificationValidator(
				const typename TNotificationFacade::NotificationType& notification,
				const ValidatorContext& context) {
			TNotificationFacade notificationFacade(notification, context.Resolvers);

			const auto& cache = context.Cache.sub<cache::MosaicRestrictionCache>();
			auto entryIter = cache.find(notificationFacade.uniqueKey());

			typename TNotificationFacade::RuleType rule;
			if (entryIter.tryGet() && notificationFacade.tryGet(entryIter.get(), rule)) {
				if (!notificationFacade.isMatch(rule))
					return Failure_RestrictionMosaic_Previous_Value_Mismatch;
			} else {
				if (!notificationFacade.isUnset())
					return Failure_RestrictionMosaic_Previous_Value_Must_Be_Zero;
			}

			return ValidationResult::Success;
		}
	}

#define DEFINE_MOSAIC_RESTRICTION_MODIFICATION_VALIDATOR(NAME) \
	DEFINE_STATEFUL_VALIDATOR_WITH_TYPE(Mosaic##NAME##RestrictionModification, NAME##NotificationFacade::NotificationType, ([]( \
			const NAME##NotificationFacade::NotificationType& notification, \
			const ValidatorContext& context) { \
		return MosaicRestrictionModificationValidator<NAME##NotificationFacade>(notification, context); \
	}))

	DEFINE_MOSAIC_RESTRICTION_MODIFICATION_VALIDATOR(Global)
	DEFINE_MOSAIC_RESTRICTION_MODIFICATION_VALIDATOR(Address)
}}
