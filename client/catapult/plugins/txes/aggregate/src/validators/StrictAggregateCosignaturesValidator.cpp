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
#include <unordered_map>

namespace catapult { namespace validators {

	using Notification = model::AggregateCosignaturesNotification;

	DEFINE_STATELESS_VALIDATOR(StrictAggregateCosignatures, [](const Notification& notification) {
		// collect all cosignatories (initially set used flag to false)
		utils::ArrayPointerFlagMap<Key> cosignatories;
		cosignatories.emplace(&notification.SignerPublicKey, false);
		const auto* pCosignature = notification.CosignaturesPtr;
		for (auto i = 0u; i < notification.CosignaturesCount; ++i) {
			cosignatories.emplace(&pCosignature->SignerPublicKey, false);
			++pCosignature;
		}

		// check all transaction signers and mark cosignatories as used
		// notice that ineligible cosignatories must dominate missing cosignatures in order for cosignatory aggregation to work
		auto hasMissingCosignatures = false;
		const auto* pTransaction = notification.TransactionsPtr;
		for (auto i = 0u; i < notification.TransactionsCount; ++i) {
			auto iter = cosignatories.find(&pTransaction->SignerPublicKey);
			if (cosignatories.cend() == iter)
				hasMissingCosignatures = true;
			else
				iter->second = true;

			pTransaction = model::AdvanceNext(pTransaction);
		}

		// only return success if all cosignatories are used
		return std::all_of(cosignatories.cbegin(), cosignatories.cend(), [](const auto& pair) { return pair.second; })
				? hasMissingCosignatures ? Failure_Aggregate_Missing_Cosignatures : ValidationResult::Success
				: Failure_Aggregate_Ineligible_Cosignatories;
	});
}}
