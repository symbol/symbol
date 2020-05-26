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

namespace catapult { namespace importance {

	namespace {
		class RestoreImportanceCalculator final : public ImportanceCalculator {
		public:
			void recalculate(model::ImportanceHeight importanceHeight, cache::AccountStateCacheDelta& cache) const override {
				const auto& highValueAddresses = cache.highValueAccounts().addresses();
				for (const auto& address : highValueAddresses) {
					auto accountStateIter = cache.find(address);
					auto& accountState = accountStateIter.get();
					if (importanceHeight < accountState.ImportanceSnapshots.height())
						accountState.ImportanceSnapshots.pop();

					auto& buckets = accountState.ActivityBuckets;
					if (buckets.end() != buckets.begin() && importanceHeight <= buckets.begin()->StartHeight)
						buckets.pop();
				}

				CATAPULT_LOG(debug) << "restored importances at height " << importanceHeight;
			}
		};
	}

	std::unique_ptr<ImportanceCalculator> CreateRestoreImportanceCalculator() {
		return std::make_unique<RestoreImportanceCalculator>();
	}
}}
