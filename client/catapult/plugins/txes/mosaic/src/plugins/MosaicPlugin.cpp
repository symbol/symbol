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

#include "MosaicPlugin.h"
#include "MosaicDefinitionTransactionPlugin.h"
#include "MosaicSupplyChangeTransactionPlugin.h"
#include "src/cache/MosaicCache.h"
#include "src/cache/MosaicCacheStorage.h"
#include "src/config/MosaicConfiguration.h"
#include "src/model/MosaicReceiptType.h"
#include "src/observers/Observers.h"
#include "src/validators/Validators.h"
#include "catapult/observers/ObserverUtils.h"
#include "catapult/observers/RentalFeeObserver.h"
#include "catapult/plugins/CacheHandlers.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace plugins {

	namespace {
		MosaicRentalFeeConfiguration ToMosaicRentalFeeConfiguration(
				const model::NetworkInfo& network,
				UnresolvedMosaicId currencyMosaicId,
				const config::MosaicConfiguration& config) {
			MosaicRentalFeeConfiguration rentalFeeConfig;
			rentalFeeConfig.CurrencyMosaicId = currencyMosaicId;
			rentalFeeConfig.Fee = config.MosaicRentalFee;
			rentalFeeConfig.NemesisSignerPublicKey = network.NemesisSignerPublicKey;

			// sink address is already resolved but needs to be passed as unresolved into notification
			rentalFeeConfig.SinkAddress = config.MosaicRentalFeeSinkAddress.copyTo<UnresolvedAddress>();
			return rentalFeeConfig;
		}

		auto GetMosaicView(const cache::CatapultCache& cache) {
			return cache.sub<cache::MosaicCache>().createView();
		}
	}

	void RegisterMosaicSubsystem(PluginManager& manager) {
		auto config = model::LoadPluginConfiguration<config::MosaicConfiguration>(manager.config(), "catapult.plugins.mosaic");
		auto unresolvedCurrencyMosaicId = model::GetUnresolvedCurrencyMosaicId(manager.config());
		auto rentalFeeConfig = ToMosaicRentalFeeConfiguration(manager.config().Network, unresolvedCurrencyMosaicId, config);
		manager.addTransactionSupport(CreateMosaicDefinitionTransactionPlugin(rentalFeeConfig));
		manager.addTransactionSupport(CreateMosaicSupplyChangeTransactionPlugin());

		manager.addCacheSupport<cache::MosaicCacheStorage>(
				std::make_unique<cache::MosaicCache>(manager.cacheConfig(cache::MosaicCache::Name)));

		using CacheHandlers = CacheHandlers<cache::MosaicCacheDescriptor>;
		CacheHandlers::Register<model::FacilityCode::Mosaic>(manager);

		manager.addDiagnosticCounterHook([](auto& counters, const cache::CatapultCache& cache) {
			counters.emplace_back(utils::DiagnosticCounterId("MOSAIC C"), [&cache]() { return GetMosaicView(cache)->size(); });
		});

		manager.addStatelessValidatorHook([](auto& builder) {
			builder
				.add(validators::CreateMosaicFlagsValidator())
				.add(validators::CreateMosaicIdValidator())
				.add(validators::CreateMosaicSupplyChangeValidator());
		});

		auto maxDuration = config.MaxMosaicDuration.blocks(manager.config().BlockGenerationTargetTime);
		auto maxAtomicUnits = manager.config().MaxMosaicAtomicUnits;
		manager.addStatefulValidatorHook([config, maxDuration, maxAtomicUnits, unresolvedCurrencyMosaicId](auto& builder) {
			builder
				.add(validators::CreateRequiredMosaicValidator())
				.add(validators::CreateMosaicAvailabilityValidator())
				.add(validators::CreateMosaicDivisibilityValidator(config.MaxMosaicDivisibility))
				.add(validators::CreateMosaicDurationValidator(maxDuration))
				.add(validators::CreateMosaicTransferValidator(unresolvedCurrencyMosaicId))
				.add(validators::CreateMaxMosaicsBalanceTransferValidator(config.MaxMosaicsPerAccount))
				.add(validators::CreateMaxMosaicsSupplyChangeValidator(config.MaxMosaicsPerAccount))
				// note that the following validator depends on RequiredMosaicValidator
				.add(validators::CreateMosaicSupplyChangeAllowedValidator(maxAtomicUnits));
		});

		auto currencyMosaicId = manager.config().CurrencyMosaicId;
		const auto& calculator = manager.inflationConfig().InflationCalculator;
		auto maxRollbackBlocks = BlockDuration(manager.config().MaxRollbackBlocks);
		manager.addObserverHook([currencyMosaicId, calculator, maxRollbackBlocks](auto& builder) {
			auto rentalFeeReceiptType = model::Receipt_Type_Mosaic_Rental_Fee;
			auto expiryReceiptType = model::Receipt_Type_Mosaic_Expired;
			builder
				.add(observers::CreateMosaicDefinitionObserver())
				.add(observers::CreateMosaicSupplyChangeObserver())
				.add(observers::CreateMosaicSupplyInflationObserver(currencyMosaicId, calculator))
				.add(observers::CreateRentalFeeObserver<model::MosaicRentalFeeNotification>("Mosaic", rentalFeeReceiptType))
				.add(observers::CreateCacheBlockTouchObserver<cache::MosaicCache>("Mosaic", expiryReceiptType));
		});
	}
}}

extern "C" PLUGIN_API
void RegisterSubsystem(catapult::plugins::PluginManager& manager) {
	catapult::plugins::RegisterMosaicSubsystem(manager);
}
