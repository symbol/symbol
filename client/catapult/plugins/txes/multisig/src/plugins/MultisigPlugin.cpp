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

#include "MultisigPlugin.h"
#include "src/cache/MultisigCache.h"
#include "src/cache/MultisigCacheStorage.h"
#include "src/config/MultisigConfiguration.h"
#include "src/handlers/MultisigDiagnosticHandlers.h"
#include "src/observers/Observers.h"
#include "src/plugins/ModifyMultisigAccountTransactionPlugin.h"
#include "src/validators/Validators.h"
#include "catapult/handlers/CacheEntryInfosProducerFactory.h"
#include "catapult/handlers/StatePathHandlerFactory.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace plugins {

	void RegisterMultisigSubsystem(PluginManager& manager) {
		manager.addTransactionSupport(CreateModifyMultisigAccountTransactionPlugin());

		manager.addCacheSupport<cache::MultisigCacheStorage>(
				std::make_unique<cache::MultisigCache>(manager.cacheConfig(cache::MultisigCache::Name)));

		manager.addDiagnosticHandlerHook([](auto& handlers, const cache::CatapultCache& cache) {
			using MultisigInfosProducerFactory = handlers::CacheEntryInfosProducerFactory<cache::MultisigCacheDescriptor>;
			handlers::RegisterMultisigInfosHandler(handlers, MultisigInfosProducerFactory::Create(cache.sub<cache::MultisigCache>()));

			using PacketType = handlers::StatePathRequestPacket<ionet::PacketType::Multisig_State_Path, Key>;
			handlers::RegisterStatePathHandler<PacketType>(handlers, cache.sub<cache::MultisigCache>());
		});

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
