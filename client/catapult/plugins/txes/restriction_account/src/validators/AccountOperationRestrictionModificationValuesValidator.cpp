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

	using Notification = model::ModifyAccountOperationRestrictionsNotification;

	namespace {
		bool AreAllAccountRestrictionValuesValid(const model::EntityType* pEntityTypes, size_t numEntityTypes) {
			return std::all_of(pEntityTypes, pEntityTypes + numEntityTypes, [](auto entityType) {
				return model::BasicEntityType::Transaction == model::ToBasicEntityType(entityType);
			});
		}
	}

	DEFINE_STATELESS_VALIDATOR(AccountOperationRestrictionModificationValues, [](const Notification& notification) {
		return AreAllAccountRestrictionValuesValid(notification.RestrictionAdditionsPtr, notification.RestrictionAdditionsCount)
				? ValidationResult::Success
				: Failure_RestrictionAccount_Invalid_Value;
	})
}}
