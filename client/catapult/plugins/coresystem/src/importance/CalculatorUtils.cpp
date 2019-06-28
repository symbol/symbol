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

#include "CalculatorUtils.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/state/AccountActivityBuckets.h"
#include "catapult/state/AccountState.h"
#include <boost/multiprecision/cpp_int.hpp>

namespace catapult { namespace importance {

	// region ImportanceHeightFacade

	namespace {
		class ImportanceHeightFacade {
		public:
			ImportanceHeightFacade(model::ImportanceHeight height, Height::ValueType importanceGrouping)
					: m_height(height)
					, m_importanceGrouping(importanceGrouping) {
				CheckHeight(m_height, m_importanceGrouping, "search");
			}

		public:
			model::ImportanceHeight previous(size_t count) const {
				auto heightAdjustment = model::ImportanceHeight(m_importanceGrouping * count);
				return m_height > heightAdjustment ? m_height - heightAdjustment : model::ImportanceHeight(1);
			}

			void checkBucketHeight(model::ImportanceHeight bucketHeight) const {
				CheckHeight(bucketHeight, m_importanceGrouping, "bucket");

				if (bucketHeight > m_height) {
					std::ostringstream out;
					out << "bucket start height " << bucketHeight<< " cannot be greater than search height " << m_height;
					CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
				}
			}

		private:
			static void CheckHeight(model::ImportanceHeight height, Height::ValueType importanceGrouping, const char* message) {
				auto isHeightGroupingMultiple = 0 == height.unwrap() % importanceGrouping;
				if (model::ImportanceHeight(1) == height || (model::ImportanceHeight(0) != height && isHeightGroupingMultiple))
					return;

				std::ostringstream out;
				out
						<< message << " importance height " << height
						<< " is inconsistent with importance grouping " << importanceGrouping;
				CATAPULT_THROW_INVALID_ARGUMENT(out.str().c_str());
			}

		private:
			model::ImportanceHeight m_height;
			Height::ValueType m_importanceGrouping;
		};
	}

	AccountActivitySummary SummarizeAccountActivity(
			model::ImportanceHeight height,
			Height::ValueType importanceGrouping,
			const state::AccountActivityBuckets& buckets) {
		ImportanceHeightFacade heightFacade(height, importanceGrouping);

		// if height is 1 (representing the first importance group), there is no previous height so set it to 0 to avoid matching
		auto previousHeight = model::ImportanceHeight(1) == height ? model::ImportanceHeight(0) : heightFacade.previous(1);
		auto minHeight = heightFacade.previous(Activity_Bucket_History_Size - Rollback_Buffer_Size - 1);

		auto activitySummary = AccountActivitySummary();
		for (const auto& bucket : buckets) {
			if (bucket.StartHeight < minHeight)
				break;

			heightFacade.checkBucketHeight(bucket.StartHeight);

			activitySummary.TotalFeesPaid = activitySummary.TotalFeesPaid + bucket.TotalFeesPaid;
			activitySummary.BeneficiaryCount += bucket.BeneficiaryCount;

			if (previousHeight == bucket.StartHeight)
				activitySummary.PreviousImportance = Importance(bucket.RawScore);
		}

		return activitySummary;
	}

	// endregion

	// region FinalizeAccountActivity

	void FinalizeAccountActivity(model::ImportanceHeight height, Importance importance, state::AccountActivityBuckets& buckets) {
		buckets.update(height, [importance](auto& bucket) {
			bucket.RawScore = importance.unwrap();
		});
	}

	// endregion

	// region CalculateImportances

	void CalculateImportances(
			AccountSummary& accountSummary,
			const ImportanceCalculationContext& context,
			const model::BlockChainConfiguration& config) {
		// note that at least one compiler is known to produce invalid code if you alter calculations in incorrect way
		auto totalChainImportance = config.TotalChainImportance;
		auto mosaicId = config.HarvestingMosaicId;
		auto importanceActivityPercentage = config.ImportanceActivityPercentage;
		auto minHarvesterBalance = config.MinHarvesterBalance;

		// 1. stake
		boost::multiprecision::uint128_t stakeImportance = totalChainImportance.unwrap();
		stakeImportance *= accountSummary.pAccountState->Balances.get(mosaicId).unwrap();
		stakeImportance *= (100 - importanceActivityPercentage);
		stakeImportance /= context.ActiveHarvestingMosaics.unwrap() * 100;
		accountSummary.StakeImportance = Importance(static_cast<Importance::ValueType>(stakeImportance));

		// 2. fees paid: importanceActivityPercentage * (minHarvesterBalance / stake) * 0.8 * feePercentage
		boost::multiprecision::uint128_t feeImportance(0);
		if (0 < importanceActivityPercentage && 0u < context.TotalFeesPaid.unwrap()) {
			feeImportance = totalChainImportance.unwrap();
			feeImportance *= accountSummary.ActivitySummary.TotalFeesPaid.unwrap();
			feeImportance *= (importanceActivityPercentage * minHarvesterBalance.unwrap() * 8);
			feeImportance /= context.TotalFeesPaid.unwrap() * 1'000;
			feeImportance /= accountSummary.pAccountState->Balances.get(mosaicId).unwrap();
		}

		// 3. beneficiary count: importanceActivityPercentage * (minHarvesterBalance / stake) * 0.2 * beneficiaryCountPercentage
		boost::multiprecision::uint128_t beneficiaryCountImportance(0);
		if (0 < importanceActivityPercentage && 0u < context.TotalBeneficiaryCount) {
			beneficiaryCountImportance = totalChainImportance.unwrap();
			beneficiaryCountImportance *= accountSummary.ActivitySummary.BeneficiaryCount;
			beneficiaryCountImportance *= (importanceActivityPercentage * minHarvesterBalance.unwrap() * 2);
			beneficiaryCountImportance /= context.TotalBeneficiaryCount * 1'000;
			beneficiaryCountImportance /= accountSummary.pAccountState->Balances.get(mosaicId).unwrap();
		}

		auto rawActivityImportance = static_cast<Importance::ValueType>(feeImportance + beneficiaryCountImportance);
		accountSummary.ActivityImportance = Importance(rawActivityImportance);
	}

	// endregion
}}
