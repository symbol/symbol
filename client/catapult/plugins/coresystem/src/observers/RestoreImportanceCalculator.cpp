#include "ImportanceCalculator.h"
#include "catapult/cache_core/AccountStateCache.h"

namespace catapult { namespace observers {

	namespace {
		class RestoreImportanceCalculator final : public ImportanceCalculator {
		public:
			void recalculate(model::ImportanceHeight importanceHeight, cache::AccountStateCacheDelta& cache) const override {
				auto highValueAddresses = cache.highValueAddresses();
				for (const auto& address : highValueAddresses) {
					auto& accountState = cache.get(address);
					if (importanceHeight < accountState.ImportanceInfo.height())
						accountState.ImportanceInfo.pop();
				}
			}
		};
	}

	std::unique_ptr<ImportanceCalculator> CreateRestoreImportanceCalculator() {
		return std::make_unique<RestoreImportanceCalculator>();
	}
}}
