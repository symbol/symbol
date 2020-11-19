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

#include "MosaicRestrictionPlugin.h"
#include "MosaicAddressRestrictionTransactionPlugin.h"
#include "MosaicGlobalRestrictionTransactionPlugin.h"
#include "src/cache/MosaicRestrictionCache.h"
#include "src/cache/MosaicRestrictionCacheStorage.h"
#include "src/config/MosaicRestrictionConfiguration.h"
#include "src/observers/Observers.h"
#include "src/validators/Validators.h"
#include "catapult/plugins/CacheHandlers.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace plugins {

	void RegisterMosaicRestrictionSubsystem(PluginManager& manager) {
		manager.addTransactionSupport(CreateMosaicAddressRestrictionTransactionPlugin());
		manager.addTransactionSupport(CreateMosaicGlobalRestrictionTransactionPlugin());

		auto networkIdentifier = manager.config().Network.Identifier;
		manager.addCacheSupport<cache::MosaicRestrictionCacheStorage>(std::make_unique<cache::MosaicRestrictionCache>(
				manager.cacheConfig(cache::MosaicRestrictionCache::Name),
				networkIdentifier));

		using CacheHandlers = CacheHandlers<cache::MosaicRestrictionCacheDescriptor>;
		CacheHandlers::Register<model::FacilityCode::RestrictionMosaic>(manager);

		manager.addDiagnosticCounterHook([](auto& counters, const cache::CatapultCache& cache) {
			counters.emplace_back(utils::DiagnosticCounterId("MOSAICREST C"), [&cache]() {
				return cache.sub<cache::MosaicRestrictionCache>().createView()->size();
			});
		});

		manager.addStatelessValidatorHook([](auto& builder) {
			builder.add(validators::CreateMosaicRestrictionTypeValidator());
		});

		auto config = model::LoadPluginConfiguration<config::MosaicRestrictionConfiguration>(
				manager.config(),
				"catapult.plugins.restrictionmosaic");
		manager.addStatefulValidatorHook([maxMosaicRestrictionValues = config.MaxMosaicRestrictionValues](auto& builder) {
			builder
				.add(validators::CreateMosaicRestrictionBalanceDebitValidator())
				.add(validators::CreateMosaicRestrictionBalanceTransferValidator())
				.add(validators::CreateMosaicRestrictionRequiredValidator())
				.add(validators::CreateMosaicGlobalRestrictionMaxValuesValidator(maxMosaicRestrictionValues))
				.add(validators::CreateMosaicGlobalRestrictionModificationValidator())
				.add(validators::CreateMosaicAddressRestrictionMaxValuesValidator(maxMosaicRestrictionValues))
				.add(validators::CreateMosaicAddressRestrictionModificationValidator());
		});

		manager.addObserverHook([](auto& builder) {
			builder
				.add(observers::CreateMosaicGlobalRestrictionCommitModificationObserver())
				.add(observers::CreateMosaicGlobalRestrictionRollbackModificationObserver())
				.add(observers::CreateMosaicAddressRestrictionCommitModificationObserver())
				.add(observers::CreateMosaicAddressRestrictionRollbackModificationObserver());
		});
	}
}}

extern "C" PLUGIN_API
void RegisterSubsystem(catapult::plugins::PluginManager& manager) {
	catapult::plugins::RegisterMosaicRestrictionSubsystem(manager);
}
