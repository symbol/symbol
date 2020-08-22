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

#include "ImportanceCalculator.h"
#include "CalculatorUtils.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/model/HeightGrouping.h"
#include "catapult/state/AccountImportanceSnapshots.h"
#include "catapult/utils/StackLogger.h"
#include <boost/multiprecision/cpp_int.hpp>
#include <memory>
#include <vector>

namespace catapult { namespace importance {

	namespace {
		class PosImportanceCalculator final : public ImportanceCalculator {
		public:
			explicit PosImportanceCalculator(const model::BlockChainConfiguration& config) : m_config(config)
			{}

		public:
			void recalculate(model::ImportanceHeight importanceHeight, cache::AccountStateCacheDelta& cache) const override {
				utils::StackLogger stopwatch("PosImportanceCalculator::recalculate", utils::LogLevel::debug);

				// 1. get high value accounts (notice two step lookup because only const iteration is supported)
				const auto& highValueAccounts = cache.highValueAccounts();
				const auto& highValueAddresses = highValueAccounts.addresses();
				std::vector<AccountSummary> accountSummaries;
				accountSummaries.reserve(highValueAddresses.size());

				// 2. calculate sums
				auto importanceGrouping = m_config.ImportanceGrouping;
				ImportanceCalculationContext context;
				auto mosaicId = m_config.HarvestingMosaicId;
				for (const auto& address : highValueAddresses) {
					auto accountStateIter = cache.find(address);
					auto& accountState = accountStateIter.get();
					const auto& activityBuckets = accountState.ActivityBuckets;
					auto accountActivitySummary = SummarizeAccountActivity(importanceHeight, importanceGrouping, activityBuckets);
					accountSummaries.push_back(AccountSummary(accountActivitySummary, accountState));
					context.ActiveHarvestingMosaics = context.ActiveHarvestingMosaics + accountState.Balances.get(mosaicId);
					context.TotalBeneficiaryCount += accountActivitySummary.BeneficiaryCount;
					context.TotalFeesPaid = context.TotalFeesPaid + accountActivitySummary.TotalFeesPaid;
				}

				// 3. calculate importance parts
				Importance totalActivityImportance;
				for (auto& accountSummary : accountSummaries) {
					CalculateImportances(accountSummary, context, m_config);
					totalActivityImportance = totalActivityImportance + accountSummary.ActivityImportance;
				}

				// 4. calculate the final importance
				auto targetActivityImportanceRaw = m_config.TotalChainImportance.unwrap() * m_config.ImportanceActivityPercentage / 100;
				for (auto& accountSummary : accountSummaries) {
					auto importance = calculateFinalImportance(accountSummary, totalActivityImportance, targetActivityImportanceRaw);
					auto& accountState = *accountSummary.pAccountState;
					FinalizeAccountActivity(importanceHeight, importance, accountState.ActivityBuckets);
					auto effectiveImportance = model::ImportanceHeight(1) == importanceHeight
							? importance
							: Importance(std::min(importance.unwrap(), accountSummary.ActivitySummary.PreviousImportance.unwrap()));
					accountSummary.pAccountState->ImportanceSnapshots.set(effectiveImportance, importanceHeight);
				}

				CATAPULT_LOG(debug) << "recalculated importances (" << highValueAddresses.size() << " / " << cache.size() << " eligible)";

				// 5. disable collection of activity for the removed accounts
				const auto& removedHighValueAddresses = highValueAccounts.removedAddresses();
				for (const auto& address : removedHighValueAddresses) {
					auto accountStateIter = cache.find(address);
					if (!accountStateIter.tryGet())
						continue;

					auto& accountState = accountStateIter.get();
					auto& activityBuckets = accountState.ActivityBuckets;
					auto currentBucket = activityBuckets.get(importanceHeight);
					if (currentBucket.StartHeight == importanceHeight)
						activityBuckets.pop();
				}
			}

		private:
			Importance calculateFinalImportance(
					const AccountSummary& accountSummary,
					Importance totalActivityImportance,
					Importance::ValueType targetActivityImportanceRaw) const {
				if (Importance() == totalActivityImportance) {
					return 0 < m_config.ImportanceActivityPercentage
							? Importance(accountSummary.StakeImportance.unwrap() * 100 / (100 - m_config.ImportanceActivityPercentage))
							: accountSummary.StakeImportance;
				}

				auto numerator = accountSummary.ActivityImportance.unwrap() * targetActivityImportanceRaw;
				return accountSummary.StakeImportance + Importance(numerator / totalActivityImportance.unwrap());
			}

		private:
			const model::BlockChainConfiguration m_config;
		};
	}

	std::unique_ptr<ImportanceCalculator> CreateImportanceCalculator(const model::BlockChainConfiguration& config) {
		return std::make_unique<PosImportanceCalculator>(config);
	}
}}
