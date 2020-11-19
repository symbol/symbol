/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "Observers.h"
#include "src/importance/ImportanceCalculator.h"
#include "catapult/cache_core/AccountStateCache.h"

namespace catapult { namespace observers {

	namespace {
		Height GetEffectiveHeight(const ObserverContext& context) {
			// note: mode commit: the hit validation for block B with height 360 is done before the block
			//                    is executed. The execution of the parent block (height 359, which passes
			//                    height 360 to the calculator) has already triggered the importance calculation
			//                    at height 359. The calculation is done on the account states seen after all
			//                    relevant block action has been executed (the calculation sees the balances
			//                    after block 359). Thus the hit validation for the block B already sees the
			//                    new importance values.
			//       summary: hit validation for block 359 uses importance values from height 1.
			//                hit validation for block 360 uses importance values from height 359.
			//
			//       mode rollback: rolling back to block height 359 (block with height 360 was the last
			//                      block rolled back), the last height passed to recalculate was 360 giving
			//                      an importance height of 359, thus a block hit validation for a new block
			//                      with height 360 will use the importance values from height 359.
			//                      rolling back to block height 358 (block with height 359 was the last
			//                      block rolled back), the last height passed to recalculate was 359 giving
			//                      an importance height of 1 which triggers a recalculation for height 1,
			//                      thus a block hit validation for a new block with height 359 will use
			//                      importances values from height 1.
			//       summary: hit validation for (a new) block 359 uses importance values from height 1.
			//                hit validation for (a new) block 360 uses importance values from height 359.
			return NotifyMode::Commit == context.Mode ? context.Height + Height(1) : context.Height;
		}

		importance::ImportanceRollbackMode GetImportanceRollbackMode(const cache::CatapultCacheDelta& delta) {
			return cache::CatapultCacheDelta::Disposition::Attached == delta.disposition()
					? importance::ImportanceRollbackMode::Enabled
					: importance::ImportanceRollbackMode::Disabled;
		}

		class RecalculateImportancesObserver : public NotificationObserverT<model::BlockNotification> {
		public:
			RecalculateImportancesObserver(
					std::unique_ptr<importance::ImportanceCalculator>&& pCommitCalculator,
					std::unique_ptr<importance::ImportanceCalculator>&& pRollbackCalculator)
					: m_pCommitCalculator(std::move(pCommitCalculator))
					, m_pRollbackCalculator(std::move(pRollbackCalculator))
					, m_name("RecalculateImportancesObserver")
			{}

		public:
			const std::string& name() const override {
				return m_name;
			}

			void notify(const model::BlockNotification&, ObserverContext& context) const override {
				auto& cache = context.Cache.sub<cache::AccountStateCache>();
				auto importanceHeight = model::ConvertToImportanceHeight(GetEffectiveHeight(context), cache.importanceGrouping());

				auto& lastRecalculationHeight = context.Cache.dependentState().LastRecalculationHeight;
				if (importanceHeight == lastRecalculationHeight)
					return;

				const auto& calculator = *(NotifyMode::Commit == context.Mode ? m_pCommitCalculator : m_pRollbackCalculator);
				calculator.recalculate(GetImportanceRollbackMode(context.Cache), importanceHeight, cache);
				lastRecalculationHeight = importanceHeight;
			}

		private:
			std::unique_ptr<importance::ImportanceCalculator> m_pCommitCalculator;
			std::unique_ptr<importance::ImportanceCalculator> m_pRollbackCalculator;
			std::string m_name;
		};
	}

	DECLARE_OBSERVER(RecalculateImportances, model::BlockNotification)(
			std::unique_ptr<importance::ImportanceCalculator>&& pCommitCalculator,
			std::unique_ptr<importance::ImportanceCalculator>&& pRollbackCalculator) {
		return std::make_unique<RecalculateImportancesObserver>(std::move(pCommitCalculator), std::move(pRollbackCalculator));
	}
}}
