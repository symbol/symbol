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

#include "AccountRestrictionPlugin.h"
#include "src/cache/AccountRestrictionCache.h"
#include "src/cache/AccountRestrictionCacheStorage.h"
#include "src/config/AccountRestrictionConfiguration.h"
#include "src/observers/Observers.h"
#include "src/plugins/AccountRestrictionTransactionPlugin.h"
#include "src/validators/Validators.h"
#include "catapult/plugins/CacheHandlers.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace plugins {

	void RegisterAccountRestrictionSubsystem(PluginManager& manager) {
		manager.addTransactionSupport(CreateAccountAddressRestrictionTransactionPlugin());
		manager.addTransactionSupport(CreateAccountMosaicRestrictionTransactionPlugin());
		manager.addTransactionSupport(CreateAccountOperationRestrictionTransactionPlugin());

		auto networkIdentifier = manager.config().Network.Identifier;
		manager.addCacheSupport<cache::AccountRestrictionCacheStorage>(std::make_unique<cache::AccountRestrictionCache>(
				manager.cacheConfig(cache::AccountRestrictionCache::Name),
				networkIdentifier));

		using CacheHandlers = CacheHandlers<cache::AccountRestrictionCacheDescriptor>;
		CacheHandlers::Register<model::FacilityCode::RestrictionAccount>(manager);

		manager.addDiagnosticCounterHook([](auto& counters, const cache::CatapultCache& cache) {
			counters.emplace_back(utils::DiagnosticCounterId("ACCTREST C"), [&cache]() {
				return cache.sub<cache::AccountRestrictionCache>().createView()->size();
			});
		});

		manager.addStatelessValidatorHook([](auto& builder) {
			builder
				.add(validators::CreateAccountRestrictionFlagsValidator())

				.add(validators::CreateAccountOperationRestrictionModificationValuesValidator());
		});

		auto config = model::LoadPluginConfiguration<config::AccountRestrictionConfiguration>(
				manager.config(),
				"catapult.plugins.restrictionaccount");
		manager.addStatefulValidatorHook([maxAccountRestrictionValues = config.MaxAccountRestrictionValues](auto& builder) {
			builder
				.add(validators::CreateAccountAddressRestrictionRedundantModificationValidator())
				.add(validators::CreateAccountAddressRestrictionValueModificationValidator())
				.add(validators::CreateMaxAccountAddressRestrictionValuesValidator(maxAccountRestrictionValues))
				.add(validators::CreateAddressInteractionValidator())
				.add(validators::CreateAccountAddressRestrictionNoSelfModificationValidator())

				.add(validators::CreateAccountMosaicRestrictionRedundantModificationValidator())
				.add(validators::CreateAccountMosaicRestrictionValueModificationValidator())
				.add(validators::CreateMaxAccountMosaicRestrictionValuesValidator(maxAccountRestrictionValues))
				.add(validators::CreateMosaicRecipientValidator())

				.add(validators::CreateAccountOperationRestrictionRedundantModificationValidator())
				.add(validators::CreateAccountOperationRestrictionValueModificationValidator())
				.add(validators::CreateMaxAccountOperationRestrictionValuesValidator(maxAccountRestrictionValues))
				.add(validators::CreateOperationRestrictionValidator())
				.add(validators::CreateAccountOperationRestrictionNoSelfBlockingValidator());
		});

		manager.addObserverHook([forkHeight = manager.config().ForkHeights.AccountRestrictionsV2](auto& builder) {
			builder
				.add(observers::CreateAccountAddressRestrictionValueModificationObserver(forkHeight))
				.add(observers::CreateAccountMosaicRestrictionValueModificationObserver(forkHeight))
				.add(observers::CreateAccountOperationRestrictionValueModificationObserver(forkHeight));
		});
	}
}}

extern "C" PLUGIN_API
void RegisterSubsystem(catapult::plugins::PluginManager& manager) {
	catapult::plugins::RegisterAccountRestrictionSubsystem(manager);
}
