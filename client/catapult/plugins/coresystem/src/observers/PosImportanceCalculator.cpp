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
			explicit PosImportanceCalculator(const model::BlockChainConfiguration& config) : m_totalChainBalance(config.TotalChainBalance)
			{}

		public:
			void recalculate(model::ImportanceHeight importanceHeight, cache::AccountStateCacheDelta& cache) const override {
				utils::StackLogger stopwatch("PosImportanceCalculator::recalculate", utils::LogLevel::Debug);

				// 1. get high value accounts (notice two step lookup because only const iteration is supported)
				auto highValueAddresses = cache.highValueAddresses();
				std::vector<state::AccountState*> highValueAccounts;
				highValueAccounts.reserve(highValueAddresses.size());

				// 2. calculate sum
				Amount activeXem;
				for (const auto& address : highValueAddresses) {
					auto accountStateIter = cache.find(address);
					auto& accountState = accountStateIter.get();
					highValueAccounts.push_back(&accountState);
					activeXem = activeXem + accountState.Balances.get(Xem_Id);
				}

				// 3. update accounts
				for (auto* pAccountState : highValueAccounts) {
					boost::multiprecision::uint128_t importance = m_totalChainBalance.microxem().unwrap();
					importance *= pAccountState->Balances.get(Xem_Id).unwrap();
					importance /= activeXem.unwrap();
					importance /= utils::XemUnit(utils::XemAmount(1)).microxem().unwrap();
					pAccountState->ImportanceInfo.set(Importance(static_cast<Importance::ValueType>(importance)), importanceHeight);
				}

				CATAPULT_LOG(debug) << "recalculated importances (" << highValueAddresses.size() << " / " << cache.size() << " eligible)";
			}

		private:
			const utils::XemUnit m_totalChainBalance;
		};
	}

	std::unique_ptr<ImportanceCalculator> CreateImportanceCalculator(const model::BlockChainConfiguration& config) {
		return std::make_unique<PosImportanceCalculator>(config);
	}
}}
