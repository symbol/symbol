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
		template<typename TUnresolved>
		static auto Resolve(const model::ResolverContext& resolvers, const TUnresolved& unresolvedValue) {
			return resolvers.resolve(unresolvedValue);
		}

		static auto Resolve(const model::ResolverContext&, const model::EntityType& unresolvedValue) {
			return unresolvedValue;
		}

		template<typename TRestrictionValue, typename TNotification>
		ValidationResult Validate(const TNotification& notification, const ValidatorContext& context) {
			auto address = model::PublicKeyToAddress(notification.Key, context.Network.Identifier);
			const auto& cache = context.Cache.sub<cache::AccountRestrictionCache>();
			if (!cache.contains(address))
				return ValidationResult::Success;

			auto restrictionsIter = cache.find(address);
			const auto& restrictions = restrictionsIter.get();
			auto restrictionType = notification.AccountRestrictionDescriptor.directionalRestrictionType();
			auto typedRestriction = restrictions.template restriction<TRestrictionValue>(restrictionType);
			auto operationType = notification.AccountRestrictionDescriptor.operationType();

			auto modification = model::AccountRestrictionModification<TRestrictionValue>{
				notification.Modification.ModificationAction,
				Resolve(context.Resolvers, notification.Modification.Value)
			};

			using OperationType = state::AccountRestrictionOperationType;
			auto isAllowAndForbidden = OperationType::Allow == operationType && !typedRestriction.canAllow(modification);
			auto isBlockAndForbidden = OperationType::Block == operationType && !typedRestriction.canBlock(modification);
			return isAllowAndForbidden || isBlockAndForbidden
					? Failure_RestrictionAccount_Invalid_Modification
					: ValidationResult::Success;
		}
	}

#define DEFINE_ACCOUNT_RESTRICTION_MODIFICATION_VALIDATOR(VALIDATOR_NAME, NOTIFICATION_TYPE, RESTRICTION_VALUE_TYPE) \
	DEFINE_STATEFUL_VALIDATOR_WITH_TYPE(VALIDATOR_NAME, NOTIFICATION_TYPE, ([]( \
			const NOTIFICATION_TYPE& notification, \
			const ValidatorContext& context) { \
		return Validate<RESTRICTION_VALUE_TYPE, NOTIFICATION_TYPE>(notification, context); \
	}));

	DEFINE_ACCOUNT_RESTRICTION_MODIFICATION_VALIDATOR(
			AccountAddressRestrictionValueModification,
			model::ModifyAccountAddressRestrictionValueNotification,
			Address)
	DEFINE_ACCOUNT_RESTRICTION_MODIFICATION_VALIDATOR(
			AccountMosaicRestrictionValueModification,
			model::ModifyAccountMosaicRestrictionValueNotification,
			MosaicId)
	DEFINE_ACCOUNT_RESTRICTION_MODIFICATION_VALIDATOR(
			AccountOperationRestrictionValueModification,
			model::ModifyAccountOperationRestrictionValueNotification,
			model::EntityType)
}}
