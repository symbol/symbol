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

#include "Observers.h"
#include "src/cache/AccountRestrictionCache.h"
#include "src/state/AccountRestrictionUtils.h"
#include "catapult/model/Address.h"

namespace catapult { namespace observers {

	namespace {
		model::AccountRestrictionModificationAction InvertModificationAction(
				model::AccountRestrictionModificationAction modificationAction) {
			return model::AccountRestrictionModificationAction::Add == modificationAction
					? model::AccountRestrictionModificationAction::Del
					: model::AccountRestrictionModificationAction::Add;
		}

		template<typename TUnresolved>
		static auto Resolve(const model::ResolverContext& resolvers, const TUnresolved& unresolvedValue) {
			return resolvers.resolve(unresolvedValue);
		}

		static auto Resolve(const model::ResolverContext&, const model::EntityType& unresolvedValue) {
			return unresolvedValue;
		}

		template<typename TNotification>
		void ObserveNotification(const TNotification& notification, const ObserverContext& context, Height compactFormatForkHeight) {
			auto& restrictionCache = context.Cache.sub<cache::AccountRestrictionCache>();
			const auto& address = notification.Address;

			auto restrictionsIter = restrictionCache.find(address);
			if (!restrictionsIter.tryGet()) {
				restrictionCache.insert(state::AccountRestrictions(address));
				restrictionsIter = restrictionCache.find(address);
			}

			auto& restrictions = restrictionsIter.get();
			restrictions.setVersion(context.Height <= compactFormatForkHeight ? 1 : 2);

			auto& restriction = restrictions.restriction(notification.AccountRestrictionDescriptor.directionalRestrictionFlags());
			auto modificationAction = NotifyMode::Commit == context.Mode
					? notification.Action
					: InvertModificationAction(notification.Action);
			auto resolvedRawValue = state::ToVector(Resolve(context.Resolvers, notification.RestrictionValue));
			model::AccountRestrictionModification rawModification{ modificationAction, resolvedRawValue };

			if (state::AccountRestrictionOperationType::Allow == notification.AccountRestrictionDescriptor.operationType())
				restriction.allow(rawModification);
			else
				restriction.block(rawModification);

			if (restrictions.isEmpty())
				restrictionCache.remove(address);
		}
	}

#define DEFINE_ACCOUNT_RESTRICTION_MODIFICATION_OBSERVER(RESTRICTION_VALUE_NAME) \
	DECLARE_OBSERVER(Account##RESTRICTION_VALUE_NAME##Modification, model::ModifyAccount##RESTRICTION_VALUE_NAME##Notification)( \
			Height compactFormatForkHeight) { \
		return MAKE_OBSERVER( \
				Account##RESTRICTION_VALUE_NAME##Modification, \
				model::ModifyAccount##RESTRICTION_VALUE_NAME##Notification, \
				([compactFormatForkHeight]( \
						const model::ModifyAccount##RESTRICTION_VALUE_NAME##Notification& notification, \
						const ObserverContext& context) { \
							ObserveNotification<model::ModifyAccount##RESTRICTION_VALUE_NAME##Notification>( \
									notification, \
									context, \
									compactFormatForkHeight); \
				})); \
	}

	DEFINE_ACCOUNT_RESTRICTION_MODIFICATION_OBSERVER(AddressRestrictionValue)
	DEFINE_ACCOUNT_RESTRICTION_MODIFICATION_OBSERVER(MosaicRestrictionValue)
	DEFINE_ACCOUNT_RESTRICTION_MODIFICATION_OBSERVER(OperationRestrictionValue)
}}
