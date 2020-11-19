/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "MultisigPlugin.h"
#include "src/cache/MultisigCache.h"
#include "src/cache/MultisigCacheStorage.h"
#include "src/config/MultisigConfiguration.h"
#include "src/observers/Observers.h"
#include "src/plugins/MultisigAccountModificationTransactionPlugin.h"
#include "src/validators/Validators.h"
#include "catapult/plugins/CacheHandlers.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace plugins {

	void RegisterMultisigSubsystem(PluginManager& manager) {
		manager.addTransactionSupport(CreateMultisigAccountModificationTransactionPlugin());

		manager.addCacheSupport<cache::MultisigCacheStorage>(
				std::make_unique<cache::MultisigCache>(manager.cacheConfig(cache::MultisigCache::Name)));

		using CacheHandlers = CacheHandlers<cache::MultisigCacheDescriptor>;
		CacheHandlers::Register<model::FacilityCode::Multisig>(manager);

		manager.addDiagnosticCounterHook([](auto& counters, const cache::CatapultCache& cache) {
			counters.emplace_back(utils::DiagnosticCounterId("MULTISIG C"), [&cache]() {
				return cache.sub<cache::MultisigCache>().createView()->size();
			});
		});

		manager.addStatelessValidatorHook([](auto& builder) {
			builder.add(validators::CreateMultisigCosignatoriesValidator());
		});

		auto config = model::LoadPluginConfiguration<config::MultisigConfiguration>(manager.config(), "catapult.plugins.multisig");
		manager.addStatefulValidatorHook([config, &transactionRegistry = manager.transactionRegistry()](auto& builder) {
			builder
				.add(validators::CreateMultisigPermittedOperationValidator())
				.add(validators::CreateMultisigMaxCosignedAccountsValidator(config.MaxCosignedAccountsPerAccount))
				.add(validators::CreateMultisigMaxCosignatoriesValidator(config.MaxCosignatoriesPerAccount))
				.add(validators::CreateMultisigInvalidCosignatoriesValidator())
				.add(validators::CreateMultisigInvalidSettingsValidator())
				// notice that MultisigLoopAndLevelValidator must be called before multisig aggregate validators
				.add(validators::CreateMultisigLoopAndLevelValidator(config.MaxMultisigDepth))
				// notice that ineligible cosignatories must dominate missing cosignatures in order for cosignatory aggregation to work
				.add(validators::CreateMultisigAggregateEligibleCosignatoriesValidator(transactionRegistry))
				.add(validators::CreateMultisigAggregateSufficientCosignatoriesValidator(transactionRegistry));
		});

		manager.addObserverHook([](auto& builder) {
			// notice that MultisigCosignatoriesObserver must be called before MultisigSettingsObserver because
			// the MultisigSettingsObserver interprets a missing entry in the multisig cache for the notification signer
			// as conversion from a multisig to a normal account done by the MultisigCosignatoriesObserver
			builder
				.add(observers::CreateMultisigCosignatoriesObserver())
				.add(observers::CreateMultisigSettingsObserver());
		});
	}
}}

extern "C" PLUGIN_API
void RegisterSubsystem(catapult::plugins::PluginManager& manager) {
	catapult::plugins::RegisterMultisigSubsystem(manager);
}
