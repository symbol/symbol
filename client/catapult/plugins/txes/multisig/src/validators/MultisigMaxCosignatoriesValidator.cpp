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

	DECLARE_STATEFUL_VALIDATOR(MultisigMaxCosignatories, Notification)(uint32_t maxCosignatoriesPerAccount) {
		return MAKE_STATEFUL_VALIDATOR(MultisigMaxCosignatories, [maxCosignatoriesPerAccount](
				const Notification& notification,
				const ValidatorContext& context) {
			const auto& multisigCache = context.Cache.sub<cache::MultisigCache>();
			auto multisigIter = multisigCache.find(notification.Multisig);

			size_t numCosignatories = 0u;
			if (multisigIter.tryGet()) {
				const auto& multisigAccountEntry = multisigIter.get();
				numCosignatories = multisigAccountEntry.cosignatoryAddresses().size();
			}

			// cannot underflow because other validator checks that all addresses being deleted exist
			numCosignatories += notification.AddressAdditionsCount - notification.AddressDeletionsCount;
			return numCosignatories > maxCosignatoriesPerAccount ? Failure_Multisig_Max_Cosignatories : ValidationResult::Success;
		});
	}
}}
