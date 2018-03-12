#include "Observers.h"
#include "ImportanceCalculator.h"
#include "catapult/cache_core/AccountStateCache.h"

namespace catapult { namespace observers {

	namespace {
		static Height GetEffectiveHeight(const ObserverContext& context) {
			// note: mode commit: the hit validation for block B with height 360 is done before the block
			//                    is executed. The execution of the parent block (height 359, which passes
			//                    height 360 to the calculator) has already triggered the importance calculation
			//                    at height 359. The calculation is done on the account states seen after all
			//                    relevant block action has been executed (the calculation sees the xem balances
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

		class RecalculateImportancesObserver : public NotificationObserverT<model::BlockNotification> {
		public:
			explicit RecalculateImportancesObserver(
					std::unique_ptr<ImportanceCalculator>&& pCommitCalculator,
					std::unique_ptr<ImportanceCalculator>&& pRollbackCalculator)
					: m_pCommitCalculator(std::move(pCommitCalculator))
					, m_pRollbackCalculator(std::move(pRollbackCalculator))
					, m_name("RecalculateImportancesObserver")
			{}

		public:
			const std::string& name() const override {
				return m_name;
			}

			void notify(const model::BlockNotification&, const ObserverContext& context) const override {
				auto& cache = context.Cache.sub<cache::AccountStateCache>();
				auto importanceHeight = model::ConvertToImportanceHeight(GetEffectiveHeight(context), cache.importanceGrouping());
				if (importanceHeight == context.State.LastRecalculationHeight)
					return;

				const auto& calculator = *(NotifyMode::Commit == context.Mode ? m_pCommitCalculator : m_pRollbackCalculator);
				calculator.recalculate(importanceHeight, cache);
				context.State.LastRecalculationHeight = importanceHeight;
			}

		private:
			std::unique_ptr<ImportanceCalculator> m_pCommitCalculator;
			std::unique_ptr<ImportanceCalculator> m_pRollbackCalculator;
			std::string m_name;
		};
	}

	DECLARE_OBSERVER(RecalculateImportances, model::BlockNotification)(
			std::unique_ptr<ImportanceCalculator>&& pCommitCalculator,
			std::unique_ptr<ImportanceCalculator>&& pRollbackCalculator) {
		return std::make_unique<RecalculateImportancesObserver>(std::move(pCommitCalculator), std::move(pRollbackCalculator));
	}
}}
