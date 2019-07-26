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
#include "src/state/AccountRestrictions.h"

namespace catapult { namespace validators {

	using Notification = model::AccountRestrictionTypeNotification;

	namespace {
		bool IsValidAccountRestrictionType(model::AccountRestrictionType restrictionType) {
			auto strippedRestrictionType = state::AccountRestrictionDescriptor(restrictionType).restrictionType();
			auto directionalRestrictionType = state::AccountRestrictionDescriptor(restrictionType).directionalRestrictionType();
			switch (strippedRestrictionType) {
			case model::AccountRestrictionType::Address:
				return true;

			case model::AccountRestrictionType::MosaicId:
				return HasSingleFlag(directionalRestrictionType);

			case model::AccountRestrictionType::TransactionType:
				return HasFlag(model::AccountRestrictionType::Outgoing, directionalRestrictionType);

			default:
				return false;
			}
		}
	}

	DEFINE_STATELESS_VALIDATOR(AccountRestrictionType, [](const Notification& notification) {
		return IsValidAccountRestrictionType(notification.RestrictionType)
				? ValidationResult::Success
				: Failure_RestrictionAccount_Invalid_Restriction_Type;
	});
}}
