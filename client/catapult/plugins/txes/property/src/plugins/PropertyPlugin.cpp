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

#include "PropertyPlugin.h"
#include "src/cache/PropertyCache.h"
#include "src/cache/PropertyCacheStorage.h"
#include "src/config/PropertyConfiguration.h"
#include "src/handlers/AccountPropertiesDiagnosticHandlers.h"
#include "src/observers/Observers.h"
#include "src/plugins/PropertyTransactionPlugin.h"
#include "src/validators/Validators.h"
#include "catapult/handlers/CacheEntryInfosProducerFactory.h"
#include "catapult/handlers/StatePathHandlerFactory.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace plugins {

	void RegisterPropertySubsystem(PluginManager& manager) {
		manager.addTransactionSupport(CreateAddressPropertyTransactionPlugin());
		manager.addTransactionSupport(CreateMosaicPropertyTransactionPlugin());
		manager.addTransactionSupport(CreateTransactionTypePropertyTransactionPlugin());

		auto networkIdentifier = manager.config().Network.Identifier;
		manager.addCacheSupport<cache::PropertyCacheStorage>(
				std::make_unique<cache::PropertyCache>(manager.cacheConfig(cache::PropertyCache::Name), networkIdentifier));

		manager.addDiagnosticHandlerHook([](auto& handlers, const cache::CatapultCache& cache) {
			using AccountPropertiesInfosProducerFactory = handlers::CacheEntryInfosProducerFactory<cache::PropertyCacheDescriptor>;

			handlers::RegisterAccountPropertiesInfosHandler(
					handlers,
					AccountPropertiesInfosProducerFactory::Create(cache.sub<cache::PropertyCache>()));

			using PacketType = handlers::StatePathRequestPacket<ionet::PacketType::Account_Properties_State_Path, Address>;
			handlers::RegisterStatePathHandler<PacketType>(handlers, cache.sub<cache::PropertyCache>());
		});

		manager.addDiagnosticCounterHook([](auto& counters, const cache::CatapultCache& cache) {
			counters.emplace_back(utils::DiagnosticCounterId("PROPERTY C"), [&cache]() {
				return cache.sub<cache::PropertyCache>().createView()->size();
			});
		});

		manager.addStatelessValidatorHook([networkIdentifier](auto& builder) {
			builder
				.add(validators::CreatePropertyTypeValidator())
				.add(validators::CreateAddressPropertyModificationTypesValidator())
				.add(validators::CreatePropertyAddressNoSelfModificationValidator(networkIdentifier))
				.add(validators::CreateMosaicPropertyModificationTypesValidator())
				.add(validators::CreateTransactionTypePropertyModificationTypesValidator())
				.add(validators::CreateTransactionTypePropertyModificationValuesValidator());
		});

		auto config = model::LoadPluginConfiguration<config::PropertyConfiguration>(manager.config(), "catapult.plugins.property");
		manager.addStatefulValidatorHook([maxPropertyValues = config.MaxPropertyValues](auto& builder) {
			builder
				.add(validators::CreateAddressPropertyRedundantModificationValidator())
				.add(validators::CreateAddressPropertyValueModificationValidator())
				.add(validators::CreateMaxAddressPropertyValuesValidator(maxPropertyValues))
				.add(validators::CreateAddressInteractionValidator())
				.add(validators::CreateMosaicPropertyRedundantModificationValidator())
				.add(validators::CreateMosaicPropertyValueModificationValidator())
				.add(validators::CreateMaxMosaicPropertyValuesValidator(maxPropertyValues))
				.add(validators::CreateMosaicRecipientValidator())
				.add(validators::CreateTransactionTypePropertyRedundantModificationValidator())
				.add(validators::CreateTransactionTypePropertyValueModificationValidator())
				.add(validators::CreateMaxTransactionTypePropertyValuesValidator(maxPropertyValues))
				.add(validators::CreateTransactionTypeValidator())
				.add(validators::CreateTransactionTypeNoSelfBlockingValidator());
		});

		manager.addObserverHook([](auto& builder) {
			builder
				.add(observers::CreateAddressPropertyValueModificationObserver())
				.add(observers::CreateMosaicPropertyValueModificationObserver())
				.add(observers::CreateTransactionTypePropertyValueModificationObserver());
		});
	}
}}

extern "C" PLUGIN_API
void RegisterSubsystem(catapult::plugins::PluginManager& manager) {
	catapult::plugins::RegisterPropertySubsystem(manager);
}
