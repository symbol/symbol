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
#include "src/cache/MultisigCache.h"
#include "catapult/utils/Hashers.h"
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::ModifyMultisigCosignersNotification;

	DEFINE_STATEFUL_VALIDATOR(ModifyMultisigInvalidCosigners, [](const auto& notification, const ValidatorContext& context) {
		const auto& multisigCache = context.Cache.sub<cache::MultisigCache>();
		const auto* pModifications = notification.ModificationsPtr;

		if (!multisigCache.contains(notification.Signer)) {
			for (auto i = 0u; i < notification.ModificationsCount; ++i) {
				if (model::CosignatoryModificationType::Del == pModifications[i].ModificationType)
					return Failure_Multisig_Modify_Unknown_Multisig_Account;
			}

			return ValidationResult::Success;
		}

		const auto& multisigEntry = multisigCache.get(notification.Signer);
		for (auto i = 0u; i < notification.ModificationsCount; ++i) {
			auto isCosignatory = multisigEntry.hasCosignatory(pModifications[i].CosignatoryPublicKey);
			auto isEnablingCosignatory = model::CosignatoryModificationType::Add == pModifications[i].ModificationType;

			if (!isEnablingCosignatory && !isCosignatory)
				return Failure_Multisig_Modify_Not_A_Cosigner;

			if (isEnablingCosignatory && isCosignatory)
				return Failure_Multisig_Modify_Already_A_Cosigner;
		}

		return ValidationResult::Success;
	});
}}
