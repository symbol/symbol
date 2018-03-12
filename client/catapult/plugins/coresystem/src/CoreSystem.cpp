#include "CoreSystem.h"
#include "handlers/CoreDiagnosticHandlers.h"
#include "observers/Observers.h"
#include "validators/Validators.h"
#include "catapult/cache_core/AccountStateCacheStorage.h"
#include "catapult/cache_core/BlockDifficultyCacheStorage.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/observers/ObserverUtils.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace plugins {

	namespace {
		cache::AccountStateCacheTypes::Options CreateAccountStateCacheOptions(const model::BlockChainConfiguration& config) {
			return { config.Network.Identifier, config.ImportanceGrouping, config.MinHarvesterBalance };
		}

		void AddAccountStateCache(PluginManager& manager, const model::BlockChainConfiguration& config) {
			using namespace catapult::cache;

			auto accountStateCacheOptions = CreateAccountStateCacheOptions(config);
			manager.addCacheSupport<AccountStateCacheStorage>(std::make_unique<AccountStateCache>(accountStateCacheOptions));

			manager.addDiagnosticHandlerHook([](auto& handlers, const CatapultCache& cache) {
				handlers::RegisterAccountInfosHandler(handlers, handlers::CreateAccountInfosSupplier(cache.sub<AccountStateCache>()));
			});

			manager.addDiagnosticCounterHook([](auto& counters, const CatapultCache& cache) {
				counters.emplace_back(utils::DiagnosticCounterId("ACNTST C"), [&cache]() {
					return cache.sub<AccountStateCache>().createView()->size();
				});
			});
		}

		void AddBlockDifficultyCache(PluginManager& manager, const model::BlockChainConfiguration& config) {
			using namespace catapult::cache;

			auto difficultyHistorySize = CalculateDifficultyHistorySize(config);
			manager.addCacheSupport<BlockDifficultyCacheStorage>(std::make_unique<BlockDifficultyCache>(difficultyHistorySize));

			manager.addDiagnosticCounterHook([](auto& counters, const CatapultCache& cache) {
				counters.emplace_back(utils::DiagnosticCounterId("BLKDIF C"), [&cache]() {
					return cache.sub<BlockDifficultyCache>().createView()->size();
				});
			});
		}
	}

	void RegisterCoreSystem(PluginManager& manager) {
		const auto& config = manager.config();

		AddAccountStateCache(manager, config);
		AddBlockDifficultyCache(manager, config);

		manager.addStatelessValidatorHook([&config](auto& builder) {
			builder
				.add(validators::CreateMaxTransactionsValidator(config.MaxTransactionsPerBlock))
				.add(validators::CreateAddressValidator(config.Network.Identifier))
				.add(validators::CreateNetworkValidator(config.Network.Identifier));
		});

		manager.addStatefulValidatorHook([&config](auto& builder) {
			builder
				.add(validators::CreateDeadlineValidator(config.MaxTransactionLifetime))
				.add(validators::CreateNemesisSinkValidator())
				.add(validators::CreateEligibleHarvesterValidator(config.MinHarvesterBalance))
				.add(validators::CreateBalanceReserveValidator())
				.add(validators::CreateBalanceTransferValidator());
		});

		manager.addObserverHook([&config](auto& builder) {
			builder
				.add(observers::CreateAccountAddressObserver())
				.add(observers::CreateAccountPublicKeyObserver())
				.add(observers::CreateBalanceObserver())
				.add(observers::CreateHarvestFeeObserver());
		});

		manager.addTransientObserverHook([&config](auto& builder) {
			auto pRecalculateImportancesObserver = observers::CreateRecalculateImportancesObserver(
					observers::CreateImportanceCalculator(config),
					observers::CreateRestoreImportanceCalculator());
			builder
				.add(std::move(pRecalculateImportancesObserver))
				.add(observers::CreateBlockDifficultyObserver())
				.add(observers::CreateCacheBlockPruningObserver<cache::BlockDifficultyCache>(
						"BlockDifficulty",
						config.BlockPruneInterval,
						BlockDuration()));
		});
	}
}}
