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
#include "src/cache/PropertyCache.h"
#include "catapult/model/Address.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	namespace {
		struct ModificationCounters {
			size_t NumAdds = 0;
			size_t NumDeletes = 0;
		};

		template<typename TPropertyValue>
		ModificationCounters ExtractModificationCounters(
				const model::PropertyModification<TPropertyValue>* pModifications,
				size_t modificationsCount) {
			ModificationCounters modificationCounters;
			for (auto i = 0u; i < modificationsCount; ++i) {
				if (model::PropertyModificationType::Add == pModifications[i].ModificationType)
					++modificationCounters.NumAdds;
				else
					++modificationCounters.NumDeletes;
			}

			return modificationCounters;
		}

		template<typename TPropertyValue, typename TNotification>
		ValidationResult Validate(uint16_t maxPropertyValues, const TNotification& notification, const ValidatorContext& context) {
			if (maxPropertyValues < notification.ModificationsCount)
				return Failure_Property_Modification_Count_Exceeded;

			auto address = model::PublicKeyToAddress(notification.Key, context.Network.Identifier);
			const auto* pModifications = notification.ModificationsPtr;
			const auto& cache = context.Cache.template sub<cache::PropertyCache>();
			if (!cache.contains(address))
				return ValidationResult::Success;

			auto accountPropertiesIter = cache.find(address);
			const auto& accountProperties = accountPropertiesIter.get();
			auto typedProperty = accountProperties.template property<TPropertyValue>(notification.PropertyDescriptor.propertyType());
			auto modificationCounters = ExtractModificationCounters<TPropertyValue>(pModifications, notification.ModificationsCount);

			// note that the PropertyModificationsValidator will detect underflows
			return maxPropertyValues < typedProperty.size() + modificationCounters.NumAdds - modificationCounters.NumDeletes
					? Failure_Property_Values_Count_Exceeded
					: ValidationResult::Success;
		}
	}

#define DEFINE_PROPERTY_MAX_VALUES_VALIDATOR(VALIDATOR_NAME, NOTIFICATION_TYPE, PROPERTY_VALUE_TYPE) \
	DECLARE_STATEFUL_VALIDATOR(VALIDATOR_NAME, NOTIFICATION_TYPE)(uint16_t maxPropertyValues) { \
		using ValidatorType = stateful::FunctionalNotificationValidatorT<NOTIFICATION_TYPE>; \
		return std::make_unique<ValidatorType>(#VALIDATOR_NAME "Validator", [maxPropertyValues]( \
				const auto& notification, \
				const auto& context) { \
			return Validate<PROPERTY_VALUE_TYPE, NOTIFICATION_TYPE>(maxPropertyValues, notification, context); \
		}); \
	}

	DEFINE_PROPERTY_MAX_VALUES_VALIDATOR(MaxAddressPropertyValues, model::ModifyAddressPropertyNotification, UnresolvedAddress)
	DEFINE_PROPERTY_MAX_VALUES_VALIDATOR(MaxMosaicPropertyValues, model::ModifyMosaicPropertyNotification, UnresolvedMosaicId)
	DEFINE_PROPERTY_MAX_VALUES_VALIDATOR(
			MaxTransactionTypePropertyValues,
			model::ModifyTransactionTypePropertyNotification,
			model::EntityType)
}}
