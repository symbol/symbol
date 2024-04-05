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

#include "SecretLockPlugin.h"
#include "src/cache/SecretLockInfoCache.h"
#include "src/config/SecretLockConfiguration.h"
#include "src/model/SecretLockReceiptType.h"
#include "src/observers/Observers.h"
#include "src/plugins/SecretLockTransactionPlugin.h"
#include "src/plugins/SecretProofTransactionPlugin.h"
#include "src/validators/Validators.h"
#include "catapult/plugins/CacheHandlers.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace plugins {

	void RegisterSecretLockSubsystem(PluginManager& manager) {
		manager.addTransactionSupport(CreateSecretProofTransactionPlugin());
		manager.addTransactionSupport(CreateSecretLockTransactionPlugin());

		manager.addCacheSupport<cache::SecretLockInfoCacheStorage>(
				std::make_unique<cache::SecretLockInfoCache>(manager.cacheConfig(cache::SecretLockInfoCache::Name)));

		using CacheHandlers = CacheHandlers<cache::SecretLockInfoCacheDescriptor>;
		CacheHandlers::Register<model::FacilityCode::LockSecret>(manager);

		manager.addDiagnosticCounterHook([](auto& counters, const cache::CatapultCache& cache) {
			counters.emplace_back(utils::DiagnosticCounterId("SECRETLOCK C"), [&cache]() {
				return cache.sub<cache::SecretLockInfoCache>().createView()->size();
			});
		});

		auto config = model::LoadPluginConfiguration<config::SecretLockConfiguration>(manager.config(), "catapult.plugins.locksecret");
		auto blockGenerationTargetTime = manager.config().BlockGenerationTargetTime;
		manager.addStatelessValidatorHook([config, blockGenerationTargetTime](auto& builder) {
			// secret lock validators
			auto maxSecretLockDuration = config.MaxSecretLockDuration.blocks(blockGenerationTargetTime);
			builder.add(validators::CreateSecretLockDurationValidator(maxSecretLockDuration));
			builder.add(validators::CreateSecretLockHashAlgorithmValidator());

			// proof secret
			builder.add(validators::CreateProofSecretValidator(config.MinProofSize, config.MaxProofSize));
		});

		auto skipUniquenessForkHeights = manager.config().ForkHeights.SkipSecretLockUniquenessChecks;
		manager.addStatefulValidatorHook([skipUniquenessForkHeights](auto& builder) {
			builder
				.add(validators::CreateSecretLockCacheUniqueValidator(skipUniquenessForkHeights))
				.add(validators::CreateProofValidator());
		});

		auto skipExpirationForkHeights = manager.config().ForkHeights.SkipSecretLockExpirations;
		auto forceExpirationForkHeights = manager.config().ForkHeights.ForceSecretLockExpirations;
		manager.addObserverHook([skipExpirationForkHeights, forceExpirationForkHeights](auto& builder) {
			builder
				.add(observers::CreateSecretLockObserver())
				.add(observers::CreateExpiredSecretLockInfoObserver(skipExpirationForkHeights, forceExpirationForkHeights))
				.add(observers::CreateProofObserver());
		});
	}
}}

extern "C" PLUGIN_API
void RegisterSubsystem(catapult::plugins::PluginManager& manager) {
	catapult::plugins::RegisterSecretLockSubsystem(manager);
}
