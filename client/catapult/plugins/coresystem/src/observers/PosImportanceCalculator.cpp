#include "ImportanceCalculator.h"
#include "src/cache/AccountStateCache.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/model/ImportanceHeight.h"
#include "catapult/state/AccountImportance.h"
#include <boost/multiprecision/cpp_int.hpp>
#include <memory>

namespace catapult { namespace observers {
	namespace {
		constexpr uint64_t Microxem_Per_Xem = 1'000'000;

		bool ShouldIncludeInImportanceCalculation(const state::AccountState& state, Amount minBalance) {
			return minBalance <= state.Balances.get(Xem_Id);
		}

		Amount::ValueType GetCumulativeBalance(cache::AccountStateCacheDelta& cache, Amount minBalance) {
			Amount::ValueType sum = 0;
			// no way to use range based loop
			for (auto iter = cache.cbegin(); cache.cend() != iter; ++iter) {
				auto pState = iter->second;
				if (ShouldIncludeInImportanceCalculation(*pState, minBalance))
					sum += pState->Balances.get(Xem_Id).unwrap();
			}

			return sum;
		}

		class PosImportanceCalculator final : public ImportanceCalculator {
		public:
			explicit PosImportanceCalculator(const model::BlockChainConfiguration& config)
					: m_totalChainBalance(config.TotalChainBalance)
					, m_minBalance(config.MinHarvesterBalance)
			{}

		public:
			void recalculate(model::ImportanceHeight importanceHeight, cache::AccountStateCacheDelta& cache) const override {
				Amount::ValueType activeXem = GetCumulativeBalance(cache, m_minBalance);
				for (auto iter = cache.cbegin(); cache.cend() != iter; ++iter) {
					auto pState = iter->second;
					if (!ShouldIncludeInImportanceCalculation(*pState, m_minBalance))
						continue;

					boost::multiprecision::uint128_t importance = m_totalChainBalance.unwrap();
					importance *= pState->Balances.get(Xem_Id).unwrap();
					importance /= activeXem;
					importance /= Microxem_Per_Xem;
					auto pMutableState = cache.findAccount(pState->Address);
					pMutableState->ImportanceInfo.set(
							Importance(static_cast<Importance::ValueType>(importance)),
							importanceHeight);
				}
			}

		private:
			const Amount m_totalChainBalance;
			const Amount m_minBalance;
		};
	}

	std::unique_ptr<ImportanceCalculator> CreateImportanceCalculator(const model::BlockChainConfiguration& config) {
		return std::make_unique<PosImportanceCalculator>(config);
	}
}}
