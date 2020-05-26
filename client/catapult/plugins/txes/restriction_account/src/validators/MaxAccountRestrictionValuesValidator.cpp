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
#include "src/cache/AccountRestrictionCache.h"
#include "catapult/model/Address.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	namespace {
		template<typename TRestrictionValue, typename TNotification>
		ValidationResult Validate(
				uint16_t maxAccountRestrictionValues,
				const TNotification& notification,
				const ValidatorContext& context) {
			if (maxAccountRestrictionValues < notification.RestrictionAdditionsCount + notification.RestrictionDeletionsCount)
				return Failure_RestrictionAccount_Modification_Count_Exceeded;

			const auto& address = notification.Address;
			const auto& cache = context.Cache.sub<cache::AccountRestrictionCache>();
			if (!cache.contains(address))
				return ValidationResult::Success;

			auto restrictionsIter = cache.find(address);
			const auto& restrictions = restrictionsIter.get();
			auto restrictionFlags = notification.AccountRestrictionDescriptor.directionalRestrictionFlags();
			const auto& restriction = restrictions.restriction(restrictionFlags);

			// note that the AccountRestrictionModificationsValidator will detect underflows
			auto numValues = restriction.values().size() + notification.RestrictionAdditionsCount - notification.RestrictionDeletionsCount;
			return maxAccountRestrictionValues < numValues
					? Failure_RestrictionAccount_Values_Count_Exceeded
					: ValidationResult::Success;
		}
	}

#define DEFINE_ACCOUNT_RESTRICTION_MAX_VALUES_VALIDATOR(VALIDATOR_NAME, NOTIFICATION_TYPE, ACCOUNT_RESTRICTION_VALUE_TYPE) \
	DECLARE_STATEFUL_VALIDATOR(VALIDATOR_NAME, NOTIFICATION_TYPE)(uint16_t maxAccountRestrictionValues) { \
		using ValidatorType = stateful::FunctionalNotificationValidatorT<NOTIFICATION_TYPE>; \
		return std::make_unique<ValidatorType>(#VALIDATOR_NAME "Validator", [maxAccountRestrictionValues]( \
				const NOTIFICATION_TYPE& notification, \
				const ValidatorContext& context) { \
			return Validate<ACCOUNT_RESTRICTION_VALUE_TYPE, NOTIFICATION_TYPE>(maxAccountRestrictionValues, notification, context); \
		}); \
	}

	DEFINE_ACCOUNT_RESTRICTION_MAX_VALUES_VALIDATOR(MaxAccountAddressRestrictionValues,
			model::ModifyAccountAddressRestrictionsNotification,
			UnresolvedAddress)
	DEFINE_ACCOUNT_RESTRICTION_MAX_VALUES_VALIDATOR(MaxAccountMosaicRestrictionValues,
			model::ModifyAccountMosaicRestrictionsNotification,
			UnresolvedMosaicId)
	DEFINE_ACCOUNT_RESTRICTION_MAX_VALUES_VALIDATOR(
			MaxAccountOperationRestrictionValues,
			model::ModifyAccountOperationRestrictionsNotification,
			model::EntityType)
}}
