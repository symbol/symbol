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
		struct ModificationsInfo {
			bool HasRedundantModification = false;
			bool HasDeleteModification = false;
		};

		template<typename TRestrictionValue, typename THasher>
		ModificationsInfo ExtractModificationsInfo(
				const model::AccountRestrictionModification<TRestrictionValue>* pModifications,
				size_t modificationsCount) {
			ModificationsInfo modificationsInfo;
			std::unordered_set<TRestrictionValue, THasher> set;
			for (auto i = 0u; i < modificationsCount; ++i) {
				set.insert(pModifications[i].Value);
				if (model::AccountRestrictionModificationType::Del == pModifications[i].ModificationType)
					modificationsInfo.HasDeleteModification = true;
			}

			modificationsInfo.HasRedundantModification = set.size() != modificationsCount;
			return modificationsInfo;
		}

		template<typename TRestrictionValue, typename TNotification, typename THasher>
		ValidationResult Validate(const TNotification& notification, const ValidatorContext& context) {
			const auto* pModifications = notification.ModificationsPtr;
			auto modificationsInfo = ExtractModificationsInfo<TRestrictionValue, THasher>(pModifications, notification.ModificationsCount);
			if (modificationsInfo.HasRedundantModification)
				return Failure_RestrictionAccount_Modification_Redundant;

			auto address = model::PublicKeyToAddress(notification.Key, context.Network.Identifier);
			const auto& cache = context.Cache.template sub<cache::AccountRestrictionCache>();
			return modificationsInfo.HasDeleteModification && !cache.contains(address)
					? Failure_RestrictionAccount_Modification_Not_Allowed
					: ValidationResult::Success;
		}
	}

#define DEFINE_ACCOUNT_RESTRICTION_REDUNDANT_MODIFICATION_VALIDATOR(RESTRICTION_NAME, RESTRICTION_VALUE_TYPE, HASHER_TYPE) \
	DEFINE_STATEFUL_VALIDATOR_WITH_TYPE(RESTRICTION_NAME##RedundantModification, model::Modify##RESTRICTION_NAME##Notification, ([]( \
			const auto& notification, \
			const auto& context) { \
		return Validate<RESTRICTION_VALUE_TYPE, model::Modify##RESTRICTION_NAME##Notification, HASHER_TYPE>(notification, context); \
	}));

	DEFINE_ACCOUNT_RESTRICTION_REDUNDANT_MODIFICATION_VALIDATOR(
			AccountAddressRestriction,
			UnresolvedAddress,
			utils::ArrayHasher<UnresolvedAddress>)
	DEFINE_ACCOUNT_RESTRICTION_REDUNDANT_MODIFICATION_VALIDATOR(
			AccountMosaicRestriction,
			UnresolvedMosaicId,
			utils::BaseValueHasher<UnresolvedMosaicId>)
	DEFINE_ACCOUNT_RESTRICTION_REDUNDANT_MODIFICATION_VALIDATOR(
			AccountOperationRestriction,
			model::EntityType,
			std::hash<model::EntityType>)
}}
