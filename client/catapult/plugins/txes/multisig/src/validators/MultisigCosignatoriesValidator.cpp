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
#include "catapult/utils/ArraySet.h"
#include <unordered_set>

namespace catapult { namespace validators {

	using Notification = model::MultisigCosignatoriesNotification;

	namespace {
		constexpr bool IsValidModificationAction(model::CosignatoryModificationAction action) {
			return model::CosignatoryModificationAction::Add == action || model::CosignatoryModificationAction::Del == action;
		}
	}

	DEFINE_STATELESS_VALIDATOR(MultisigCosignatories, [](const Notification& notification) {
		utils::KeyPointerSet addedAccounts;
		utils::KeyPointerSet removedAccounts;

		const auto* pModifications = notification.ModificationsPtr;
		for (auto i = 0u; i < notification.ModificationsCount; ++i) {
			if (!IsValidModificationAction(pModifications[i].ModificationAction))
				return Failure_Multisig_Invalid_Modification_Action;

			auto& accounts = model::CosignatoryModificationAction::Add == pModifications[i].ModificationAction
					? addedAccounts
					: removedAccounts;
			const auto& oppositeAccounts = &accounts == &addedAccounts ? removedAccounts : addedAccounts;

			auto& key = pModifications[i].CosignatoryPublicKey;
			if (oppositeAccounts.end() != oppositeAccounts.find(&key))
				return Failure_Multisig_Account_In_Both_Sets;

			accounts.insert(&key);
		}

		if (1 < removedAccounts.size())
			return Failure_Multisig_Multiple_Deletes;

		if (notification.ModificationsCount != addedAccounts.size() + removedAccounts.size())
			return Failure_Multisig_Redundant_Modification;

		return ValidationResult::Success;
	});
}}
