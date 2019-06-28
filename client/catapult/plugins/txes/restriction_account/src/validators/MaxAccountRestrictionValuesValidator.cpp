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
		struct ModificationCounters {
			size_t NumAdds = 0;
			size_t NumDeletes = 0;
		};

		template<typename TRestrictionValue>
		ModificationCounters ExtractModificationCounters(
				const model::AccountRestrictionModification<TRestrictionValue>* pModifications,
				size_t modificationsCount) {
			ModificationCounters modificationCounters;
			for (auto i = 0u; i < modificationsCount; ++i) {
				if (model::AccountRestrictionModificationType::Add == pModifications[i].ModificationType)
					++modificationCounters.NumAdds;
				else
					++modificationCounters.NumDeletes;
			}

			return modificationCounters;
		}

		template<typename TRestrictionValue, typename TNotification>
		ValidationResult Validate(
				uint16_t maxAccountRestrictionValues,
				const TNotification& notification,
				const ValidatorContext& context) {
			if (maxAccountRestrictionValues < notification.ModificationsCount)
				return Failure_RestrictionAccount_Modification_Count_Exceeded;

			auto address = model::PublicKeyToAddress(notification.Key, context.Network.Identifier);
			const auto* pModifications = notification.ModificationsPtr;
			const auto& cache = context.Cache.template sub<cache::AccountRestrictionCache>();
			if (!cache.contains(address))
				return ValidationResult::Success;

			auto restrictionsIter = cache.find(address);
			const auto& restrictions = restrictionsIter.get();
			auto restrictionType = notification.AccountRestrictionDescriptor.restrictionType();
			auto typedRestriction = restrictions.template restriction<TRestrictionValue>(restrictionType);
			auto modificationCounters = ExtractModificationCounters<TRestrictionValue>(pModifications, notification.ModificationsCount);

			// note that the AccountRestrictionModificationsValidator will detect underflows
			auto numValues = typedRestriction.size() + modificationCounters.NumAdds - modificationCounters.NumDeletes;
			return maxAccountRestrictionValues < numValues
					? Failure_RestrictionAccount_Values_Count_Exceeded
					: ValidationResult::Success;
		}
	}

#define DEFINE_ACCOUNT_RESTRICTION_MAX_VALUES_VALIDATOR(VALIDATOR_NAME, NOTIFICATION_TYPE, ACCOUNT_RESTRICTION_VALUE_TYPE) \
	DECLARE_STATEFUL_VALIDATOR(VALIDATOR_NAME, NOTIFICATION_TYPE)(uint16_t maxAccountRestrictionValues) { \
		using ValidatorType = stateful::FunctionalNotificationValidatorT<NOTIFICATION_TYPE>; \
		return std::make_unique<ValidatorType>(#VALIDATOR_NAME "Validator", [maxAccountRestrictionValues]( \
				const auto& notification, \
				const auto& context) { \
			return Validate<ACCOUNT_RESTRICTION_VALUE_TYPE, NOTIFICATION_TYPE>(maxAccountRestrictionValues, notification, context); \
		}); \
	}

	DEFINE_ACCOUNT_RESTRICTION_MAX_VALUES_VALIDATOR(MaxAccountAddressRestrictionValues,
			model::ModifyAccountAddressRestrictionNotification,
			UnresolvedAddress)
	DEFINE_ACCOUNT_RESTRICTION_MAX_VALUES_VALIDATOR(MaxAccountMosaicRestrictionValues,
			model::ModifyAccountMosaicRestrictionNotification,
			UnresolvedMosaicId)
	DEFINE_ACCOUNT_RESTRICTION_MAX_VALUES_VALIDATOR(
			MaxAccountOperationRestrictionValues,
			model::ModifyAccountOperationRestrictionNotification,
			model::EntityType)
}}
