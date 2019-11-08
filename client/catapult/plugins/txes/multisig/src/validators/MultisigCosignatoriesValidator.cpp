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
		utils::KeyPointerSet ToSet(const Key* pKeys, uint8_t numKeys) {
			utils::KeyPointerSet keys;
			for (auto i = 0u; i < numKeys; ++i)
				keys.insert(&pKeys[i]);

			return keys;
		}
	}

	DEFINE_STATELESS_VALIDATOR(MultisigCosignatories, [](const Notification& notification) {
		if (1 < notification.PublicKeyDeletionsCount)
			return Failure_Multisig_Multiple_Deletes;

		auto publicKeyAdditions = ToSet(notification.PublicKeyAdditionsPtr, notification.PublicKeyAdditionsCount);
		auto publicKeyDeletions = ToSet(notification.PublicKeyDeletionsPtr, notification.PublicKeyDeletionsCount);
		if (notification.PublicKeyAdditionsCount != publicKeyAdditions.size())
			return Failure_Multisig_Redundant_Modification;

		for (const auto* pKey : publicKeyAdditions) {
			if (publicKeyDeletions.cend() != publicKeyDeletions.find(pKey))
				return Failure_Multisig_Account_In_Both_Sets;
		}

		return ValidationResult::Success;
	});
}}
