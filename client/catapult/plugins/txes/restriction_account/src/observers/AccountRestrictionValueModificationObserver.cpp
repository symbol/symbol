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
#include "src/cache/AccountRestrictionCache.h"
#include "src/state/AccountRestrictionUtils.h"
#include "catapult/model/Address.h"

namespace catapult { namespace observers {

	namespace {
		model::AccountRestrictionModificationType InvertModificationType(model::AccountRestrictionModificationType modificationType) {
			return model::AccountRestrictionModificationType::Add == modificationType
					? model::AccountRestrictionModificationType::Del
					: model::AccountRestrictionModificationType::Add;
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
			auto& restrictionCache = context.Cache.sub<cache::AccountRestrictionCache>();
			auto address = model::PublicKeyToAddress(notification.Key, restrictionCache.networkIdentifier());

			auto restrictionsIter = restrictionCache.find(address);
			if (!restrictionsIter.tryGet()) {
				restrictionCache.insert(state::AccountRestrictions(address));
				restrictionsIter = restrictionCache.find(address);
			}

			auto& restrictions = restrictionsIter.get();
			auto& restriction = restrictions.restriction(notification.AccountRestrictionDescriptor.restrictionType());
			const auto& modification = notification.Modification;
			auto modificationType = NotifyMode::Commit == context.Mode
					? modification.ModificationType
					: InvertModificationType(modification.ModificationType);
			auto resolvedRawValue = state::ToVector(Resolve(context.Resolvers, modification.Value));
			model::RawAccountRestrictionModification rawModification{ modificationType, resolvedRawValue };

			if (state::AccountRestrictionOperationType::Allow == notification.AccountRestrictionDescriptor.operationType())
				restriction.allow(rawModification);
			else
				restriction.block(rawModification);

			if (restrictions.isEmpty())
				restrictionCache.remove(address);
		}
	}

#define DEFINE_ACCOUNT_RESTRICTION_MODIFICATION_OBSERVER(RESTRICTION_VALUE_NAME) \
	DEFINE_OBSERVER(Account##RESTRICTION_VALUE_NAME##Modification, model::ModifyAccount##RESTRICTION_VALUE_NAME##Notification, []( \
				const auto& notification, \
				const ObserverContext& context) { \
		HandleNotifications<model::ModifyAccount##RESTRICTION_VALUE_NAME##Notification>(notification, context); \
	});

	DEFINE_ACCOUNT_RESTRICTION_MODIFICATION_OBSERVER(AddressRestrictionValue)
	DEFINE_ACCOUNT_RESTRICTION_MODIFICATION_OBSERVER(MosaicRestrictionValue)
	DEFINE_ACCOUNT_RESTRICTION_MODIFICATION_OBSERVER(OperationRestrictionValue)
}}
