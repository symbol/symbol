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

	using Notification = model::ModifyMultisigCosignersNotification;

	namespace {
		constexpr bool IsValidModificationType(model::CosignatoryModificationType type) {
			return model::CosignatoryModificationType::Add == type || model::CosignatoryModificationType::Del == type;
		}
	}

	DEFINE_STATELESS_VALIDATOR(ModifyMultisigCosigners, [](const Notification& notification) {
		utils::KeyPointerSet addedAccounts;
		utils::KeyPointerSet removedAccounts;

		const auto* pModifications = notification.ModificationsPtr;
		for (auto i = 0u; i < notification.ModificationsCount; ++i) {
			if (!IsValidModificationType(pModifications[i].ModificationType))
				return Failure_Multisig_Modify_Unsupported_Modification_Type;

			auto& accounts = model::CosignatoryModificationType::Add == pModifications[i].ModificationType
					? addedAccounts
					: removedAccounts;
			const auto& oppositeAccounts = &accounts == &addedAccounts ? removedAccounts : addedAccounts;

			auto& key = pModifications[i].CosignatoryPublicKey;
			if (oppositeAccounts.end() != oppositeAccounts.find(&key))
				return Failure_Multisig_Modify_Account_In_Both_Sets;

			accounts.insert(&key);
		}

		if (1 < removedAccounts.size())
			return Failure_Multisig_Modify_Multiple_Deletes;

		if (notification.ModificationsCount != addedAccounts.size() + removedAccounts.size())
			return Failure_Multisig_Modify_Redundant_Modifications;

		return ValidationResult::Success;
	});
}}
