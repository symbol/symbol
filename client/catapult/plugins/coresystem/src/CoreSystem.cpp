#include "CoreSystem.h"
#include "cache/AccountStateCacheStorage.h"
#include "handlers/CoreDiagnosticHandlers.h"
#include "observers/Observers.h"
#include "validators/Validators.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace plugins {

	void RegisterCoreSystem(PluginManager& manager) {
		const auto& config = manager.config();

		manager.addCacheSupport<cache::AccountStateCacheStorage>(
				std::make_unique<cache::AccountStateCache>(config.Network.Identifier, config.ImportanceGrouping));

		manager.addDiagnosticHandlerHook([](auto& handlers, const cache::CatapultCache& cache) {
			handlers::RegisterAccountInfosHandler(handlers, handlers::CreateAccountInfosSupplier(cache.sub<cache::AccountStateCache>()));
		});

		manager.addDiagnosticCounterHook([](auto& counters, const cache::CatapultCache& cache) {
			counters.emplace_back(utils::DiagnosticCounterId("ACNTST C"), [&cache]() {
				return cache.sub<cache::AccountStateCache>().createView()->size();
			});
		});

		manager.addStatelessValidatorHook([&config](auto& builder) {
			builder
				.add(validators::CreateNonFutureBlockValidator(config.MaxBlockFutureTime))
				.add(validators::CreateMaxTransactionsValidator(config.MaxTransactionsPerBlock))
				.add(validators::CreateNetworkValidator(config.Network.Identifier))
				.add(validators::CreateSignatureValidator());
		});

		manager.addStatefulValidatorHook([&config](auto& builder) {
			builder
				.add(validators::CreateDeadlineValidator(config.MaxTransactionLifetime))
				.add(validators::CreateNemesisSinkValidator())
				.add(validators::CreateEligibleHarvesterValidator(Amount(config.MinHarvesterBalance)))
				.add(validators::CreateBalanceValidator());
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
				.add(std::move(pRecalculateImportancesObserver));
		});
	}
}}
