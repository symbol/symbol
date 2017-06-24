#include "MemoryHashCacheSystem.h"
#include "src/cache/HashCacheStorage.h"
#include "src/handlers/HashCacheDiagnosticHandlers.h"
#include "src/observers/Observers.h"
#include "src/validators/Validators.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace plugins {

	void RegisterMemoryHashCacheSystem(PluginManager& manager) {
		const auto& config = manager.config();

		manager.addCacheSupport<cache::HashCacheStorage>(std::make_unique<cache::HashCache>(CalculateTransactionCacheDuration(config)));

		manager.addDiagnosticHandlerHook([](auto& handlers, const cache::CatapultCache& cache) {
			handlers::RegisterConfirmTimestampedHashesHandler(
					handlers,
					handlers::CreateConfirmedTimestampedHashesFilter(cache.sub<cache::HashCache>()));
		});

		manager.addDiagnosticCounterHook([](auto& counters, const cache::CatapultCache& cache) {
			counters.emplace_back(utils::DiagnosticCounterId("HASH C"), [&cache]() {
				return cache.sub<cache::HashCache>().createView()->size();
			});
		});

		manager.addStatefulValidatorHook([](auto& builder) {
			builder.add(validators::CreateUniqueTransactionHashValidator());
		});

		manager.addTransientObserverHook([&config](auto& builder) {
			builder
				.add(observers::CreateTransactionHashObserver())
				.add(observers::CreateTransactionHashPruningObserver(config.BlockPruneInterval));
		});
	}
}}

extern "C" PLUGIN_API
void RegisterSubsystem(catapult::plugins::PluginManager& manager) {
	catapult::plugins::RegisterMemoryHashCacheSystem(manager);
}
