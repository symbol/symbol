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

		template<typename TRestrictionValue, typename THasher, typename TNotification>
		ModificationsInfo ExtractModificationsInfo(const TNotification& notification) {
			auto numModifications = static_cast<size_t>(notification.RestrictionAdditionsCount + notification.RestrictionDeletionsCount);

			std::unordered_set<TRestrictionValue, THasher> set;
			for (auto i = 0u; i < notification.RestrictionAdditionsCount; ++i)
				set.insert(notification.RestrictionAdditionsPtr[i]);

			for (auto i = 0u; i < notification.RestrictionDeletionsCount; ++i)
				set.insert(notification.RestrictionDeletionsPtr[i]);

			ModificationsInfo modificationsInfo;
			modificationsInfo.HasDeleteModification = 0 != notification.RestrictionDeletionsCount;
			modificationsInfo.HasRedundantModification = set.size() != numModifications;
			return modificationsInfo;
		}

		template<typename TRestrictionValue, typename TNotification, typename THasher>
		ValidationResult Validate(const TNotification& notification, const ValidatorContext& context) {
			auto modificationsInfo = ExtractModificationsInfo<TRestrictionValue, THasher>(notification);
			if (modificationsInfo.HasRedundantModification)
				return Failure_RestrictionAccount_Redundant_Modification;

			const auto& cache = context.Cache.sub<cache::AccountRestrictionCache>();
			return modificationsInfo.HasDeleteModification && !cache.contains(notification.Address)
					? Failure_RestrictionAccount_Invalid_Modification
					: ValidationResult::Success;
		}
	}

#define DEFINE_ACCOUNT_RESTRICTION_REDUNDANT_MODIFICATION_VALIDATOR(RESTRICTION_NAME, RESTRICTION_VALUE_TYPE, HASHER_TYPE) \
	DEFINE_STATEFUL_VALIDATOR_WITH_TYPE(RESTRICTION_NAME##RedundantModification, model::Modify##RESTRICTION_NAME##sNotification, ([]( \
			const model::Modify##RESTRICTION_NAME##sNotification& notification, \
			const ValidatorContext& context) { \
		return Validate<RESTRICTION_VALUE_TYPE, model::Modify##RESTRICTION_NAME##sNotification, HASHER_TYPE>(notification, context); \
	}))

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
