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

namespace catapult { namespace validators {

	using Notification = model::AggregateCosignaturesNotification;

	DECLARE_STATELESS_VALIDATOR(BasicAggregateCosignatures, Notification)(uint32_t maxTransactions, uint8_t maxCosignatures) {
		return MAKE_STATELESS_VALIDATOR(BasicAggregateCosignatures, ([maxTransactions, maxCosignatures](const Notification& notification) {
			if (0 == notification.TransactionsCount)
				return Failure_Aggregate_No_Transactions;

			if (maxTransactions < notification.TransactionsCount)
				return Failure_Aggregate_Too_Many_Transactions;

			if (maxCosignatures < notification.CosignaturesCount + 1)
				return Failure_Aggregate_Too_Many_Cosignatures;

			utils::KeyPointerSet cosignatories;
			cosignatories.insert(&notification.SignerPublicKey);
			const auto* pCosignature = notification.CosignaturesPtr;
			for (auto i = 0u; i < notification.CosignaturesCount; ++i) {
				if (!cosignatories.insert(&pCosignature->SignerPublicKey).second)
					return Failure_Aggregate_Redundant_Cosignatures;

				++pCosignature;
			}

			return ValidationResult::Success;
		}));
	}
}}
