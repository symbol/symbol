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
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/model/ImportanceHeight.h"
#include "catapult/state/AccountImportance.h"
#include "catapult/utils/StackLogger.h"
#include <boost/multiprecision/cpp_int.hpp>
#include <memory>
#include <vector>

namespace catapult { namespace observers {

	namespace {
		class PosImportanceCalculator final : public ImportanceCalculator {
		public:
			explicit PosImportanceCalculator(Importance totalChainImportance, MosaicId harvestingMosaicId)
					: m_totalChainImportance(totalChainImportance)
					, m_harvestingMosaicId(harvestingMosaicId)
			{}

		public:
			void recalculate(model::ImportanceHeight importanceHeight, cache::AccountStateCacheDelta& cache) const override {
				utils::StackLogger stopwatch("PosImportanceCalculator::recalculate", utils::LogLevel::Debug);

				// 1. get high value accounts (notice two step lookup because only const iteration is supported)
				auto highValueAddresses = cache.highValueAddresses();
				std::vector<state::AccountState*> highValueAccounts;
				highValueAccounts.reserve(highValueAddresses.size());

				// 2. calculate sum
				Amount activeHarvestingMosaics;
				for (const auto& address : highValueAddresses) {
					auto accountStateIter = cache.find(address);
					auto& accountState = accountStateIter.get();
					highValueAccounts.push_back(&accountState);
					activeHarvestingMosaics = activeHarvestingMosaics + accountState.Balances.get(m_harvestingMosaicId);
				}

				// 3. update accounts
				for (auto* pAccountState : highValueAccounts) {
					boost::multiprecision::uint128_t importance = m_totalChainImportance.unwrap();
					importance *= pAccountState->Balances.get(m_harvestingMosaicId).unwrap();
					importance /= activeHarvestingMosaics.unwrap();
					pAccountState->ImportanceInfo.set(Importance(static_cast<Importance::ValueType>(importance)), importanceHeight);
				}

				CATAPULT_LOG(debug) << "recalculated importances (" << highValueAddresses.size() << " / " << cache.size() << " eligible)";
			}

		private:
			const Importance m_totalChainImportance;
			const MosaicId m_harvestingMosaicId;
		};
	}

	std::unique_ptr<ImportanceCalculator> CreateImportanceCalculator(const model::BlockChainConfiguration& config) {
		return std::make_unique<PosImportanceCalculator>(config.TotalChainImportance, config.HarvestingMosaicId);
	}
}}
