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

#include "HashLockPlugin.h"
#include "src/cache/HashLockInfoCache.h"
#include "src/config/HashLockConfiguration.h"
#include "src/model/HashLockReceiptType.h"
#include "src/observers/Observers.h"
#include "src/plugins/HashLockTransactionPlugin.h"
#include "src/validators/Validators.h"
#include "catapult/plugins/CacheHandlers.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace plugins {

	void RegisterHashLockSubsystem(PluginManager& manager) {
		manager.addTransactionSupport(CreateHashLockTransactionPlugin());

		manager.addCacheSupport<cache::HashLockInfoCacheStorage>(
				std::make_unique<cache::HashLockInfoCache>(manager.cacheConfig(cache::HashLockInfoCache::Name)));

		using CacheHandlers = CacheHandlers<cache::HashLockInfoCacheDescriptor>;
		CacheHandlers::Register<model::FacilityCode::LockHash>(manager);

		manager.addDiagnosticCounterHook([](auto& counters, const cache::CatapultCache& cache) {
			counters.emplace_back(utils::DiagnosticCounterId("HASHLOCK C"), [&cache]() {
				return cache.sub<cache::HashLockInfoCache>().createView()->size();
			});
		});

		auto config = model::LoadPluginConfiguration<config::HashLockConfiguration>(manager.config(), "catapult.plugins.lockhash");
		auto blockGenerationTargetTime = manager.config().BlockGenerationTargetTime;
		auto currencyMosaicId = manager.config().CurrencyMosaicId;
		manager.addStatelessValidatorHook([config, blockGenerationTargetTime](auto& builder) {
			// hash lock validators
			auto maxHashLockDuration = config.MaxHashLockDuration.blocks(blockGenerationTargetTime);
			builder.add(validators::CreateHashLockDurationValidator(maxHashLockDuration));
		});

		manager.addStatefulValidatorHook([config, currencyMosaicId](auto& builder) {
			builder
				.add(validators::CreateAggregateHashPresentValidator())
				.add(validators::CreateHashLockCacheUniqueValidator())
				.add(validators::CreateHashLockMosaicValidator(currencyMosaicId, config.LockedFundsPerAggregate));
		});

		auto maxRollbackBlocks = BlockDuration(manager.config().MaxRollbackBlocks);
		manager.addObserverHook([maxRollbackBlocks](auto& builder) {
			builder
				.add(observers::CreateHashLockObserver())
				.add(observers::CreateExpiredHashLockInfoObserver())
				.add(observers::CreateCompletedAggregateObserver());
		});
	}
}}

extern "C" PLUGIN_API
void RegisterSubsystem(catapult::plugins::PluginManager& manager) {
	catapult::plugins::RegisterHashLockSubsystem(manager);
}
