#include "MultisigPlugin.h"
#include "src/cache/MultisigCacheStorage.h"
#include "src/config/MultisigConfiguration.h"
#include "src/observers/Observers.h"
#include "src/plugins/ModifyMultisigAccountTransactionPlugin.h"
#include "src/validators/Validators.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace plugins {

	void RegisterMultisigSubsystem(PluginManager& manager) {
		manager.addTransactionSupport(CreateModifyMultisigAccountTransactionPlugin());

		manager.addCacheSupport<cache::MultisigCacheStorage>(std::make_unique<cache::MultisigCache>());

		manager.addStatelessValidatorHook([](auto& builder) {
			builder.add(validators::CreateModifyMultisigCosignersValidator());
		});

		auto config = model::LoadPluginConfiguration<config::MultisigConfiguration>(manager.config(), "catapult.plugins.multisig");
		manager.addStatefulValidatorHook([config](auto& builder) {
			builder
				.add(validators::CreateMultisigPermittedOperationValidator())
				.add(validators::CreateModifyMultisigMaxCosignedAccountsValidator(config.MaxCosignedAccountsPerAccount))
				.add(validators::CreateModifyMultisigMaxCosignersValidator(config.MaxCosignersPerAccount))
				.add(validators::CreateModifyMultisigInvalidCosignersValidator())
				.add(validators::CreateModifyMultisigInvalidSettingsValidator())
				// notice that ModifyMultisigLoopAndLevelValidator must be called before multisig aggregate validators
				.add(validators::CreateModifyMultisigLoopAndLevelValidator(config.MaxMultisigDepth))
				// notice that ineligible cosigners must dominate missing cosigners in order for cosigner aggregation to work
				.add(validators::CreateMultisigAggregateEligibleCosignersValidator())
				.add(validators::CreateMultisigAggregateSufficientCosignersValidator());
		});

		manager.addObserverHook([](auto& builder) {
			// notice that ModifyMultisigCosignersObserver must be called before ModifyMultisigSettingsObserver because
			// the ModifyMultisigSettingsObserver interprets a missing entry in the multisig cache for the notification signer
			// as conversion from a multisig to a normal account done by the ModifyMultisigCosignersObserver
			builder
				.add(observers::CreateModifyMultisigCosignersObserver())
				.add(observers::CreateModifyMultisigSettingsObserver());
		});
	}
}}

extern "C" PLUGIN_API
void RegisterSubsystem(catapult::plugins::PluginManager& manager) {
	catapult::plugins::RegisterMultisigSubsystem(manager);
}
