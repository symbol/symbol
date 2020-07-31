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
#include "AccountRestrictionView.h"
#include "src/cache/AccountRestrictionCache.h"
#include "src/model/AccountOperationRestrictionTransaction.h"
#include "catapult/model/Address.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::ModifyAccountOperationRestrictionValueNotification;

	namespace {
		constexpr auto Relevant_Entity_Type = model::AccountOperationRestrictionTransaction::Entity_Type;
		constexpr auto Restriction_Flags = model::AccountRestrictionFlags::TransactionType | model::AccountRestrictionFlags::Outgoing;

		bool Validate(const Notification& notification, const ValidatorContext& context) {
			AccountRestrictionView view(context.Cache);
			auto isRelevantEntityType = Relevant_Entity_Type == notification.RestrictionValue;
			auto isAllow = state::AccountRestrictionOperationType::Allow == notification.AccountRestrictionDescriptor.operationType();

			// cannot delete relevant entity type for operation type Allow
			if (model::AccountRestrictionModificationAction::Del == notification.Action)
				return !(isAllow && isRelevantEntityType);

			size_t numRestrictionValues = 0;
			if (view.initialize(notification.Address)) {
				const auto& restriction = view.get(Restriction_Flags);
				numRestrictionValues = restriction.values().size();
			}

			// adding a value to an account restrictions should only be allowed when
			// - operation type Allow: if it is the relevant entity type or the relevant entity type is already contained
			// - operation type Block: if it is not the relevant entity type
			auto isAllowAndForbidden = isAllow && !isRelevantEntityType && 0 == numRestrictionValues;
			auto isBlockAndForbidden = !isAllow && isRelevantEntityType;
			return !(isAllowAndForbidden || isBlockAndForbidden);
		}
	}

	DEFINE_STATEFUL_VALIDATOR(AccountOperationRestrictionNoSelfBlocking, [](
			const Notification& notification,
			const ValidatorContext& context) {
		return Validate(notification, context) ? ValidationResult::Success : Failure_RestrictionAccount_Invalid_Modification;
	})
}}
