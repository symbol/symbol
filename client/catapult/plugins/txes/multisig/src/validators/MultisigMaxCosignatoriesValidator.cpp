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
#include "catapult/validators/ValidatorContext.h"

namespace catapult { namespace validators {

	using Notification = model::MultisigCosignatoriesNotification;

	DECLARE_STATEFUL_VALIDATOR(MultisigMaxCosignatories, Notification)(uint8_t maxCosignatoriesPerAccount) {
		return MAKE_STATEFUL_VALIDATOR(MultisigMaxCosignatories, [maxCosignatoriesPerAccount](
				const Notification& notification,
				const ValidatorContext& context) {
			size_t numCosignatories = 0u;
			const auto& multisigCache = context.Cache.sub<cache::MultisigCache>();
			if (multisigCache.contains(notification.Signer)) {
				auto multisigIter = multisigCache.find(notification.Signer);
				const auto& multisigAccountEntry = multisigIter.get();
				numCosignatories = multisigAccountEntry.cosignatoryPublicKeys().size();
			}

			const auto* pCosignatoryModification = notification.ModificationsPtr;
			for (auto i = 0u; i < notification.ModificationsCount; ++i) {
				if (model::CosignatoryModificationAction::Add == pCosignatoryModification->ModificationAction)
					++numCosignatories;
				else
					--numCosignatories;

				++pCosignatoryModification;
			}

			return numCosignatories > maxCosignatoriesPerAccount ? Failure_Multisig_Max_Cosignatories : ValidationResult::Success;
		});
	}
}}
