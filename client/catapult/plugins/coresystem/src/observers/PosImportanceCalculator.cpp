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
		constexpr uint64_t Microxem_Per_Xem = 1'000'000;

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
					auto& accountState = cache.get(address);
					highValueAccounts.push_back(&accountState);
					activeXem = activeXem + accountState.Balances.get(Xem_Id);
				}

				// 3. update accounts
				for (auto* pAccountState : highValueAccounts) {
					boost::multiprecision::uint128_t importance = m_totalChainBalance.unwrap();
					importance *= pAccountState->Balances.get(Xem_Id).unwrap();
					importance /= activeXem.unwrap();
					importance /= Microxem_Per_Xem;
					pAccountState->ImportanceInfo.set(Importance(static_cast<Importance::ValueType>(importance)), importanceHeight);
				}

				CATAPULT_LOG(debug) << "recalculated importances (" << highValueAddresses.size() << " / " << cache.size() << " eligible)";
			}

		private:
			const Amount m_totalChainBalance;
		};
	}

	std::unique_ptr<ImportanceCalculator> CreateImportanceCalculator(const model::BlockChainConfiguration& config) {
		return std::make_unique<PosImportanceCalculator>(config);
	}
}}
