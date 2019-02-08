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
		struct ModificationsInfo {
			bool HasRedundantModification = false;
			bool HasDeleteModification = false;
		};

		template<typename TPropertyValue, typename THasher>
		ModificationsInfo ExtractModificationsInfo(
				const model::PropertyModification<TPropertyValue>* pModifications,
				size_t modificationsCount) {
			ModificationsInfo modificationsInfo;
			std::unordered_set<TPropertyValue, THasher> set;
			for (auto i = 0u; i < modificationsCount; ++i) {
				set.insert(pModifications[i].Value);
				if (model::PropertyModificationType::Del == pModifications[i].ModificationType)
					modificationsInfo.HasDeleteModification = true;
			}

			modificationsInfo.HasRedundantModification = set.size() != modificationsCount;
			return modificationsInfo;
		}

		template<typename TPropertyValue, typename TNotification, typename THasher>
		ValidationResult Validate(const TNotification& notification, const ValidatorContext& context) {
			const auto* pModifications = notification.ModificationsPtr;
			auto modificationsInfo = ExtractModificationsInfo<TPropertyValue, THasher>(pModifications, notification.ModificationsCount);
			if (modificationsInfo.HasRedundantModification)
				return Failure_Property_Modification_Redundant;

			auto address = model::PublicKeyToAddress(notification.Key, context.Network.Identifier);
			const auto& cache = context.Cache.template sub<cache::PropertyCache>();
			return modificationsInfo.HasDeleteModification && !cache.contains(address)
					? Failure_Property_Modification_Not_Allowed
					: ValidationResult::Success;
		}
	}

#define DEFINE_PROPERTY_REDUNDANT_MODIFICATION_VALIDATOR(VALIDATOR_NAME, NOTIFICATION_TYPE, PROPERTY_VALUE_TYPE, HASHER_TYPE) \
	DEFINE_STATEFUL_VALIDATOR_WITH_TYPE(VALIDATOR_NAME, NOTIFICATION_TYPE, ([](const auto& notification, const auto& context) { \
		return Validate<PROPERTY_VALUE_TYPE, NOTIFICATION_TYPE, HASHER_TYPE>(notification, context); \
	}));

	DEFINE_PROPERTY_REDUNDANT_MODIFICATION_VALIDATOR(
			AddressPropertyRedundantModification,
			model::ModifyAddressPropertyNotification,
			UnresolvedAddress,
			utils::UnresolvedAddressHasher)
	DEFINE_PROPERTY_REDUNDANT_MODIFICATION_VALIDATOR(
			MosaicPropertyRedundantModification,
			model::ModifyMosaicPropertyNotification,
			UnresolvedMosaicId,
			utils::BaseValueHasher<UnresolvedMosaicId>)
	DEFINE_PROPERTY_REDUNDANT_MODIFICATION_VALIDATOR(
			TransactionTypePropertyRedundantModification,
			model::ModifyTransactionTypePropertyNotification,
			model::EntityType,
			std::hash<model::EntityType>)
}}
