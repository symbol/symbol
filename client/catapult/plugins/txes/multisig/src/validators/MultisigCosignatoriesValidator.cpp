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
		utils::ArrayPointerSet<UnresolvedAddress> ToSet(const UnresolvedAddress* pAddresses, uint8_t count) {
			utils::ArrayPointerSet<UnresolvedAddress> addresses;
			for (auto i = 0u; i < count; ++i)
				addresses.insert(&pAddresses[i]);

			return addresses;
		}
	}

	// notice that redundant checks by this validator are not comprehensive and there is another validator that will
	// check against current state and resolved addresses

	DEFINE_STATELESS_VALIDATOR(MultisigCosignatories, [](const Notification& notification) {
		if (1 < notification.AddressDeletionsCount)
			return Failure_Multisig_Multiple_Deletes;

		auto addressAdditions = ToSet(notification.AddressAdditionsPtr, notification.AddressAdditionsCount);
		auto addressDeletions = ToSet(notification.AddressDeletionsPtr, notification.AddressDeletionsCount);
		if (notification.AddressAdditionsCount != addressAdditions.size())
			return Failure_Multisig_Redundant_Modification;

		for (const auto* pAddress : addressAdditions) {
			if (addressDeletions.cend() != addressDeletions.find(pAddress))
				return Failure_Multisig_Account_In_Both_Sets;
		}

		return ValidationResult::Success;
	})
}}
