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
		template<typename TUnresolved>
		static auto Resolve(const model::ResolverContext& resolvers, const TUnresolved& unresolvedValue) {
			return resolvers.resolve(unresolvedValue);
		}

		static auto Resolve(const model::ResolverContext&, const model::EntityType& unresolvedValue) {
			return unresolvedValue;
		}

		template<typename TPropertyValue, typename TNotification>
		ValidationResult Validate(const TNotification& notification, const ValidatorContext& context) {
			auto address = model::PublicKeyToAddress(notification.Key, context.Network.Identifier);
			const auto& cache = context.Cache.template sub<cache::PropertyCache>();
			if (!cache.contains(address))
				return ValidationResult::Success;

			auto accountPropertiesIter = cache.find(address);
			const auto& accountProperties = accountPropertiesIter.get();
			auto typedProperty = accountProperties.template property<TPropertyValue>(notification.PropertyDescriptor.propertyType());
			auto operationType = notification.PropertyDescriptor.operationType();

			auto modification = model::PropertyModification<TPropertyValue>{
				notification.Modification.ModificationType,
				Resolve(context.Resolvers, notification.Modification.Value)
			};
			auto isAllowAndForbidden = state::OperationType::Allow == operationType && !typedProperty.canAllow(modification);
			auto isBlockAndForbidden = state::OperationType::Block == operationType && !typedProperty.canBlock(modification);
			return isAllowAndForbidden || isBlockAndForbidden ? Failure_Property_Modification_Not_Allowed : ValidationResult::Success;
		}
	}

#define DEFINE_PROPERTY_MODIFICATION_VALIDATOR(VALIDATOR_NAME, NOTIFICATION_TYPE, PROPERTY_VALUE_TYPE) \
	DEFINE_STATEFUL_VALIDATOR_WITH_TYPE(VALIDATOR_NAME, NOTIFICATION_TYPE, ([](const auto& notification, const auto& context) { \
		return Validate<PROPERTY_VALUE_TYPE, NOTIFICATION_TYPE>(notification, context); \
	}));

	DEFINE_PROPERTY_MODIFICATION_VALIDATOR(AddressPropertyValueModification, model::ModifyAddressPropertyValueNotification, Address)
	DEFINE_PROPERTY_MODIFICATION_VALIDATOR(MosaicPropertyValueModification, model::ModifyMosaicPropertyValueNotification, MosaicId)
	DEFINE_PROPERTY_MODIFICATION_VALIDATOR(
			TransactionTypePropertyValueModification,
			model::ModifyTransactionTypePropertyValueNotification,
			model::EntityType)
}}
