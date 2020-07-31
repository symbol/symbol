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
#include "catapult/utils/Functional.h"
#include "catapult/utils/Hashers.h"
#include "catapult/utils/RawBuffer.h"
#include "catapult/validators/ValidatorContext.h"
#include <limits>

namespace catapult { namespace validators {

	using Notification = model::MultisigSettingsNotification;

	DEFINE_STATEFUL_VALIDATOR(MultisigInvalidSettings, [](const Notification& notification, const ValidatorContext& context) {
		const auto& multisigCache = context.Cache.sub<cache::MultisigCache>();
		auto multisigIter = multisigCache.find(notification.Multisig);

		if (!multisigIter.tryGet()) {
			// since the MultisigInvalidCosignatoriesValidator and the MultisigCosignatoriesObserver ran before
			// this validator, the only scenario in which the multisig account cannot be found in the multisig cache
			// is that the observer removed the last cosignatory reverting the multisig account to a normal accounts

			// MinRemovalDelta and MinApprovalDelta are both expected to be -1 in this case
			if (-1 != notification.MinRemovalDelta || -1 != notification.MinApprovalDelta)
				return Failure_Multisig_Min_Setting_Out_Of_Range;

			return ValidationResult::Success;
		}

		const auto& multisigEntry = multisigIter.get();
		int64_t newMinRemoval = static_cast<int64_t>(multisigEntry.minRemoval()) + notification.MinRemovalDelta;
		int64_t newMinApproval = static_cast<int64_t>(multisigEntry.minApproval()) + notification.MinApprovalDelta;
		if (1 > newMinRemoval || 1 > newMinApproval)
			return Failure_Multisig_Min_Setting_Out_Of_Range;

		auto maxValue = static_cast<int64_t>(multisigEntry.cosignatoryAddresses().size());
		if (newMinRemoval > maxValue || newMinApproval > maxValue)
			return Failure_Multisig_Min_Setting_Larger_Than_Num_Cosignatories;

		return ValidationResult::Success;
	})
}}
