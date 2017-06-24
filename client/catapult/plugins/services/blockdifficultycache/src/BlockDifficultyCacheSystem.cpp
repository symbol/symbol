#include "BlockDifficultyCacheSystem.h"
#include "cache/BlockDifficultyCacheStorage.h"
#include "observers/Observers.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace plugins {

	void RegisterBlockDifficultyCacheSystem(PluginManager& manager) {
		const auto& config = manager.config();

		manager.addCacheSupport<cache::BlockDifficultyCacheStorage>(
				std::make_unique<cache::BlockDifficultyCache>(CalculateDifficultyHistorySize(config)));

		manager.addDiagnosticCounterHook([](auto& counters, const cache::CatapultCache& cache) {
			counters.emplace_back(utils::DiagnosticCounterId("BLKDIF C"), [&cache]() {
				return cache.sub<cache::BlockDifficultyCache>().createView()->size();
			});
		});

		manager.addTransientObserverHook([&config](auto& builder) {
			builder
				.add(observers::CreateBlockDifficultyObserver())
				.add(observers::CreateBlockDifficultyPruningObserver(config.BlockPruneInterval));
		});
	}
}}
