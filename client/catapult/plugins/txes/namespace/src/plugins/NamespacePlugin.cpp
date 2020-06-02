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

#include "NamespacePlugin.h"
#include "AddressAliasTransactionPlugin.h"
#include "MosaicAliasTransactionPlugin.h"
#include "NamespaceRegistrationTransactionPlugin.h"
#include "src/cache/NamespaceCache.h"
#include "src/cache/NamespaceCacheStorage.h"
#include "src/cache/NamespaceCacheSubCachePlugin.h"
#include "src/config/NamespaceConfiguration.h"
#include "src/model/NamespaceLifetimeConstraints.h"
#include "src/model/NamespaceReceiptType.h"
#include "src/observers/Observers.h"
#include "src/validators/Validators.h"
#include "catapult/cache/ReadOnlyCatapultCache.h"
#include "catapult/observers/ObserverUtils.h"
#include "catapult/observers/RentalFeeObserver.h"
#include "catapult/plugins/CacheHandlers.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace plugins {

	namespace {
		// region alias

		void RegisterAliasSubsystem(PluginManager& manager, const config::NamespaceConfiguration&) {
			manager.addTransactionSupport(CreateAddressAliasTransactionPlugin());
			manager.addTransactionSupport(CreateMosaicAliasTransactionPlugin());

			manager.addStatelessValidatorHook([](auto& builder) {
				builder.add(validators::CreateAliasActionValidator());
			});

			manager.addStatefulValidatorHook([](auto& builder) {
				builder
					.add(validators::CreateAliasAvailabilityValidator())
					.add(validators::CreateUnlinkAliasedAddressConsistencyValidator())
					.add(validators::CreateUnlinkAliasedMosaicIdConsistencyValidator())
					.add(validators::CreateAddressAliasValidator());
			});

			manager.addObserverHook([](auto& builder) {
				builder
					.add(observers::CreateAliasedAddressObserver())
					.add(observers::CreateAliasedMosaicIdObserver());
			});
		}

		// endregion

		// region namespace

		NamespaceRentalFeeConfiguration ToNamespaceRentalFeeConfiguration(
				const model::NetworkInfo& network,
				UnresolvedMosaicId currencyMosaicId,
				const config::NamespaceConfiguration& config) {
			NamespaceRentalFeeConfiguration rentalFeeConfig;
			rentalFeeConfig.CurrencyMosaicId = currencyMosaicId;
			rentalFeeConfig.RootFeePerBlock = config.RootNamespaceRentalFeePerBlock;
			rentalFeeConfig.ChildFee = config.ChildNamespaceRentalFee;
			rentalFeeConfig.NemesisSignerPublicKey = network.NemesisSignerPublicKey;

			// sink address is already resolved but needs to be passed as unresolved into notification
			rentalFeeConfig.SinkAddress = config.NamespaceRentalFeeSinkAddress.copyTo<UnresolvedAddress>();
			return rentalFeeConfig;
		}

		template<typename TAliasValue, typename TAliasValueAccessor>
		bool RunNamespaceResolver(
				const cache::NamespaceCacheTypes::CacheReadOnlyType& namespaceCache,
				NamespaceId namespaceId,
				state::AliasType aliasType,
				TAliasValue& aliasValue,
				TAliasValueAccessor aliasValueAccessor) {
			auto iter = namespaceCache.find(namespaceId);
			if (!iter.tryGet())
				return false;

			const auto& alias = iter.get().root().alias(namespaceId);
			if (aliasType != alias.type())
				return false;

			aliasValue = aliasValueAccessor(alias);
			return true;
		}

		void RegisterNamespaceAliasResolvers(PluginManager& manager) {
			manager.addMosaicResolver([](const auto&, const auto& unresolved, auto& resolved) {
				constexpr uint64_t Namespace_Flag = 1ull << 63;
				if (0 == (Namespace_Flag & unresolved.unwrap())) {
					resolved = model::ResolverContext().resolve(unresolved);
					return true;
				}

				return false;
			});

			manager.addMosaicResolver([](const auto& cache, const auto& unresolved, auto& resolved) {
				auto namespaceCache = cache.template sub<cache::NamespaceCache>();
				auto namespaceId = NamespaceId(unresolved.unwrap());
				return RunNamespaceResolver(namespaceCache, namespaceId, state::AliasType::Mosaic, resolved, [](const auto& alias) {
					return alias.mosaicId();
				});
			});

			manager.addAddressResolver([](const auto&, const auto& unresolved, auto& resolved) {
				if (0 == (1 & unresolved[0])) {
					resolved = model::ResolverContext().resolve(unresolved);
					return true;
				}

				return false;
			});

			manager.addAddressResolver([](const auto& cache, const auto& unresolved, auto& resolved) {
				auto namespaceCache = cache.template sub<cache::NamespaceCache>();
				NamespaceId namespaceId;
				std::memcpy(static_cast<void*>(&namespaceId), unresolved.data() + 1, sizeof(NamespaceId));
				return RunNamespaceResolver(namespaceCache, namespaceId, state::AliasType::Address, resolved, [](const auto& alias) {
					return alias.address();
				});
			});
		}

		auto GetNamespaceView(const cache::CatapultCache& cache) {
			return cache.sub<cache::NamespaceCache>().createView();
		}

		void RegisterNamespaceSubsystem(PluginManager& manager, const config::NamespaceConfiguration& config) {
			auto currencyMosaicId = model::GetUnresolvedCurrencyMosaicId(manager.config());
			auto rentalFeeConfig = ToNamespaceRentalFeeConfiguration(manager.config().Network, currencyMosaicId, config);
			manager.addTransactionSupport(CreateNamespaceRegistrationTransactionPlugin(rentalFeeConfig));

			auto minDuration = config.MinNamespaceDuration.blocks(manager.config().BlockGenerationTargetTime);
			auto maxDuration = config.MaxNamespaceDuration.blocks(manager.config().BlockGenerationTargetTime);
			auto gracePeriodDuration = config.NamespaceGracePeriodDuration.blocks(manager.config().BlockGenerationTargetTime);
			model::NamespaceLifetimeConstraints constraints(maxDuration, gracePeriodDuration);

			RegisterNamespaceAliasResolvers(manager);
			manager.addCacheSupport(std::make_unique<cache::NamespaceCacheSubCachePlugin>(
					manager.cacheConfig(cache::NamespaceCache::Name),
					cache::NamespaceCacheTypes::Options{ gracePeriodDuration }));

			using CacheHandlers = CacheHandlers<cache::NamespaceCacheDescriptor>;
			CacheHandlers::Register<model::FacilityCode::Namespace>(manager);

			manager.addDiagnosticCounterHook([](auto& counters, const cache::CatapultCache& cache) {
				counters.emplace_back(utils::DiagnosticCounterId("NS C"), [&cache]() { return GetNamespaceView(cache)->size(); });
				counters.emplace_back(utils::DiagnosticCounterId("NS C AS"), [&cache]() { return GetNamespaceView(cache)->activeSize(); });
				counters.emplace_back(utils::DiagnosticCounterId("NS C DS"), [&cache]() { return GetNamespaceView(cache)->deepSize(); });
			});

			manager.addStatelessValidatorHook([config, minDuration, maxDuration](auto& builder) {
				const auto& reservedNames = config.ReservedRootNamespaceNames;
				builder
					.add(validators::CreateNamespaceRegistrationTypeValidator())
					.add(validators::CreateNamespaceNameValidator(config.MaxNameSize, reservedNames))
					.add(validators::CreateRootNamespaceValidator(minDuration, maxDuration));
			});

			manager.addStatefulValidatorHook([constraints, config](auto& builder) {
				builder
					.add(validators::CreateRootNamespaceAvailabilityValidator())
					.add(validators::CreateNamespaceDurationOverflowValidator(constraints.MaxNamespaceDuration))
					// note that the following validator needs to run before the RootNamespaceMaxChildrenValidator
					.add(validators::CreateChildNamespaceAvailabilityValidator(config.MaxNamespaceDepth))
					.add(validators::CreateRootNamespaceMaxChildrenValidator(config.MaxChildNamespaces))
					.add(validators::CreateRequiredNamespaceValidator());
			});

			auto maxRollbackBlocks = BlockDuration(manager.config().MaxRollbackBlocks);
			manager.addObserverHook([gracePeriodDuration, maxRollbackBlocks](auto& builder) {
				auto rentalFeeReceiptType = model::Receipt_Type_Namespace_Rental_Fee;
				auto expiryReceiptType = model::Receipt_Type_Namespace_Deleted;
				builder
					.add(observers::CreateRootNamespaceObserver())
					.add(observers::CreateChildNamespaceObserver())
					.add(observers::CreateRentalFeeObserver<model::NamespaceRentalFeeNotification>("Namespace", rentalFeeReceiptType))
					.add(observers::CreateCacheBlockTouchObserver<cache::NamespaceCache>(
							"NamespaceGracePeriod",
							model::Receipt_Type_Namespace_Expired,
							gracePeriodDuration))
					.add(observers::CreateCacheBlockTouchObserver<cache::NamespaceCache>("Namespace", expiryReceiptType));
			});
		}

		// endregion
	}

	void RegisterNamespaceSubsystem(PluginManager& manager) {
		auto config = model::LoadPluginConfiguration<config::NamespaceConfiguration>(manager.config(), "catapult.plugins.namespace");
		RegisterNamespaceSubsystem(manager, config);
		RegisterAliasSubsystem(manager, config);
	}
}}

extern "C" PLUGIN_API
void RegisterSubsystem(catapult::plugins::PluginManager& manager) {
	catapult::plugins::RegisterNamespaceSubsystem(manager);
}
