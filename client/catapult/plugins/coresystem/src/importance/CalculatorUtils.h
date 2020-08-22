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

#pragma once
#include "catapult/model/HeightGrouping.h"

namespace catapult {
	namespace model { struct BlockChainConfiguration; }
	namespace state {
		class AccountActivityBuckets;
		struct AccountState;
	}
}

namespace catapult { namespace importance {

	/// Summarized account activity information.
	struct AccountActivitySummary {
		/// Total fees paid by account.
		Amount TotalFeesPaid;

		/// Number of times account has been used as a beneficiary.
		uint32_t BeneficiaryCount = 0;

		/// Previous account importance.
		Importance PreviousImportance;
	};

	/// Summarized account information.
	struct AccountSummary {
	public:
		/// Creates an account summary around \a activitySummary and \a accountState.
		AccountSummary(const AccountActivitySummary& activitySummary, state::AccountState& accountState)
				: ActivitySummary(activitySummary)
				, pAccountState(&accountState)
		{}

	public:
		/// Account activity summary.
		AccountActivitySummary ActivitySummary;

		/// Account state.
		state::AccountState* pAccountState;

		/// Importance due to account stake.
		Importance StakeImportance;

		/// Importance due to account activity.
		Importance ActivityImportance;
	};

	/// Context for importance calculation.
	struct ImportanceCalculationContext {
	public:
		/// Total active harvesting mosaics.
		Amount ActiveHarvestingMosaics;

		/// Total beneficiary count.
		uint64_t TotalBeneficiaryCount = 0;

		/// Total fees paid.
		Amount TotalFeesPaid;

		/// Total importance due to account activity.
		Importance TotalActivityImportance;
	};

	/// Summarizes account activity information contained in \a buckets starting at \a height given specified
	/// importance grouping (\a importanceGrouping).
	AccountActivitySummary SummarizeAccountActivity(
			model::ImportanceHeight height,
			Height::ValueType importanceGrouping,
			const state::AccountActivityBuckets& buckets);

	/// Finalizes account activity information contained in \a buckets at \a height with specified \a importance.
	void FinalizeAccountActivity(model::ImportanceHeight height, Importance importance, state::AccountActivityBuckets& buckets);

	/// Calculates stake and activity importances using \a context and \a config and stores resulting importances in \a accountSummary.
	void CalculateImportances(
			AccountSummary& accountSummary,
			const ImportanceCalculationContext& context,
			const model::BlockChainConfiguration& config);
}}
