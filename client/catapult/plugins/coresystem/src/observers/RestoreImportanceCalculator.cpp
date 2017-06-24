#include "ImportanceCalculator.h"
#include "src/cache/AccountStateCache.h"

namespace catapult { namespace observers {

	namespace {
		class RestoreImportanceCalculator final : public ImportanceCalculator {
		public:
			void recalculate(model::ImportanceHeight importanceHeight, cache::AccountStateCacheDelta& cache) const override {
				for (auto iter = cache.cbegin(); cache.cend() != iter; ++iter) {
					// skip all accounts with no importances at heights greater than importanceHeight
					auto pState = iter->second;
					if (importanceHeight >= pState->ImportanceInfo.height())
						continue;

					auto pMutableState = cache.findAccount(pState->Address);
					while (importanceHeight < pMutableState->ImportanceInfo.height())
						pMutableState->ImportanceInfo.pop();
				}
			}
		};
	}

	std::unique_ptr<ImportanceCalculator> CreateRestoreImportanceCalculator() {
		return std::make_unique<RestoreImportanceCalculator>();
	}
}}
