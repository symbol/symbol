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

#include "Observers.h"
#include "src/cache/PropertyCache.h"
#include "src/state/PropertyUtils.h"
#include "catapult/model/Address.h"

namespace catapult { namespace observers {

	namespace {
		model::PropertyModificationType InvertModificationType(model::PropertyModificationType modificationType) {
			return model::PropertyModificationType::Add == modificationType
					? model::PropertyModificationType::Del
					: model::PropertyModificationType::Add;
		}

		template<typename TUnresolved>
		static auto Resolve(const model::ResolverContext& resolvers, const TUnresolved& unresolvedValue) {
			return resolvers.resolve(unresolvedValue);
		}

		static auto Resolve(const model::ResolverContext&, const model::EntityType& unresolvedValue) {
			return unresolvedValue;
		}

		template<typename TNotification>
		void HandleNotifications(const TNotification& notification, const ObserverContext& context) {
			auto& propertyCache = context.Cache.sub<cache::PropertyCache>();
			auto address = model::PublicKeyToAddress(notification.Key, propertyCache.networkIdentifier());

			auto accountPropertiesIter = propertyCache.find(address);
			if (!accountPropertiesIter.tryGet()) {
				propertyCache.insert(state::AccountProperties(address));
				accountPropertiesIter = propertyCache.find(address);
			}

			auto& accountProperties = accountPropertiesIter.get();
			auto& accountProperty = accountProperties.property(notification.PropertyDescriptor.propertyType());
			const auto& modification = notification.Modification;
			auto modificationType = NotifyMode::Commit == context.Mode
					? modification.ModificationType
					: InvertModificationType(modification.ModificationType);
			auto resolvedRawValue = state::ToVector(Resolve(context.Resolvers, modification.Value));
			model::RawPropertyModification rawModification{ modificationType, resolvedRawValue };

			if (state::OperationType::Allow == notification.PropertyDescriptor.operationType())
				accountProperty.allow(rawModification);
			else
				accountProperty.block(rawModification);

			if (accountProperties.isEmpty())
				propertyCache.remove(address);
		}
	}

#define DEFINE_PROPERTY_MODIFICATION_OBSERVER(OBSERVER_NAME, NOTIFICATION_TYPE) \
	DEFINE_OBSERVER(OBSERVER_NAME, NOTIFICATION_TYPE, [](const auto& notification, const ObserverContext& context) { \
		HandleNotifications<NOTIFICATION_TYPE>(notification, context); \
	});

	DEFINE_PROPERTY_MODIFICATION_OBSERVER(AddressPropertyValueModification, model::ModifyAddressPropertyValueNotification)
	DEFINE_PROPERTY_MODIFICATION_OBSERVER(MosaicPropertyValueModification, model::ModifyMosaicPropertyValueNotification)
	DEFINE_PROPERTY_MODIFICATION_OBSERVER(TransactionTypePropertyValueModification, model::ModifyTransactionTypePropertyValueNotification)
}}
