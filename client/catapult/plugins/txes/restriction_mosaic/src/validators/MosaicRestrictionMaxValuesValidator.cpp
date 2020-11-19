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
#include "src/cache/MosaicRestrictionCache.h"
#include "src/state/MosaicAddressRestrictionNotificationFacade.h"
#include "src/state/MosaicGlobalRestrictionNotificationFacade.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	namespace {
		using GlobalNotificationFacade = state::MosaicGlobalRestrictionNotificationFacade<
			model::MosaicGlobalRestrictionModificationNewValueNotification::Notification_Type>;
		using AddressNotificationFacade = state::MosaicAddressRestrictionNotificationFacade<
			model::MosaicAddressRestrictionModificationNewValueNotification::Notification_Type>;

		size_t GetRestrictionCount(const state::MosaicRestrictionEntry& entry) {
			return state::MosaicRestrictionEntry::EntryType::Global == entry.entryType()
					? entry.asGlobalRestriction().size()
					: entry.asAddressRestriction().size();
		}

		template<typename TNotificationFacade>
		ValidationResult MosaicRestrictionMaxValuesValidator(
				uint8_t maxMosaicRestrictionValues,
				const typename TNotificationFacade::NotificationType& notification,
				const ValidatorContext& context) {
			TNotificationFacade notificationFacade(notification, context.Resolvers);
			auto isDeleteAction = notificationFacade.isDeleteAction();

			const auto& cache = context.Cache.sub<cache::MosaicRestrictionCache>();
			auto entryIter = cache.find(notificationFacade.uniqueKey());
			if (!entryIter.tryGet())
				return isDeleteAction ? Failure_RestrictionMosaic_Cannot_Delete_Nonexistent_Restriction : ValidationResult::Success;

			typename TNotificationFacade::RuleType rule;
			auto isNewRule = !notificationFacade.tryGet(entryIter.get(), rule);
			if (isDeleteAction && isNewRule)
				return Failure_RestrictionMosaic_Cannot_Delete_Nonexistent_Restriction;

			return isDeleteAction || !isNewRule || GetRestrictionCount(entryIter.get()) < maxMosaicRestrictionValues
					? ValidationResult::Success
					: Failure_RestrictionMosaic_Max_Restrictions_Exceeded;
		}
	}

#define DEFINE_MOSAIC_RESTRICTION_MAX_VALUES_VALIDATOR(NAME) \
	DECLARE_STATEFUL_VALIDATOR(Mosaic##NAME##RestrictionMaxValues, NAME##NotificationFacade::NotificationType)( \
			uint8_t maxMosaicRestrictionValues) { \
		using ValidatorType = stateful::FunctionalNotificationValidatorT<NAME##NotificationFacade::NotificationType>; \
		return std::make_unique<ValidatorType>("Mosaic" #NAME "RestrictionMaxValuesValidator", [maxMosaicRestrictionValues]( \
				const NAME##NotificationFacade::NotificationType& notification, \
				const ValidatorContext& context) { \
			return MosaicRestrictionMaxValuesValidator<NAME##NotificationFacade>(maxMosaicRestrictionValues, notification, context); \
		}); \
	}

	DEFINE_MOSAIC_RESTRICTION_MAX_VALUES_VALIDATOR(Global)
	DEFINE_MOSAIC_RESTRICTION_MAX_VALUES_VALIDATOR(Address)
}}
