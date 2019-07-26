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

namespace catapult { namespace validators {

	namespace {
		template<typename TRestrictionValue>
		bool AreAllAccountRestrictionModificationTypesValid(
				const model::AccountRestrictionModification<TRestrictionValue>* pModifications,
				size_t modificationsCount) {
			return std::all_of(pModifications, pModifications + modificationsCount, [](const auto& modification) {
				return model::AccountRestrictionModificationType::Del >= modification.ModificationType;
			});
		}
	}

#define DEFINE_ACCOUNT_RESTRICTION_MODIFICATION_TYPE_VALIDATOR(VALIDATOR_NAME, NOTIFICATION_TYPE, RESTRICTION_VALUE_TYPE) \
	DEFINE_STATELESS_VALIDATOR_WITH_TYPE(VALIDATOR_NAME, NOTIFICATION_TYPE, [](const NOTIFICATION_TYPE& notification) { \
		auto areAllValid = AreAllAccountRestrictionModificationTypesValid<RESTRICTION_VALUE_TYPE>( \
				notification.ModificationsPtr, \
				notification.ModificationsCount); \
		return areAllValid ? ValidationResult::Success : Failure_RestrictionAccount_Modification_Type_Invalid; \
	});

	DEFINE_ACCOUNT_RESTRICTION_MODIFICATION_TYPE_VALIDATOR(
			AccountAddressRestrictionModificationTypes,
			model::ModifyAccountAddressRestrictionNotification,
			UnresolvedAddress)
	DEFINE_ACCOUNT_RESTRICTION_MODIFICATION_TYPE_VALIDATOR(
			AccountMosaicRestrictionModificationTypes,
			model::ModifyAccountMosaicRestrictionNotification,
			UnresolvedMosaicId)
	DEFINE_ACCOUNT_RESTRICTION_MODIFICATION_TYPE_VALIDATOR(
			AccountOperationRestrictionModificationTypes,
			model::ModifyAccountOperationRestrictionNotification,
			model::EntityType)
}}
