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

#include "CoreSystem.h"
#include "VotingKeyLinkTransactionPlugin.h"
#include "VrfKeyLinkTransactionPlugin.h"
#include "src/importance/ImportanceCalculator.h"
#include "src/observers/Observers.h"
#include "src/validators/Validators.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/cache_core/AccountStateCacheStorage.h"
#include "catapult/cache_core/AccountStateCacheSubCachePlugin.h"
#include "catapult/cache_core/BlockStatisticCacheStorage.h"
#include "catapult/cache_core/BlockStatisticCacheSubCachePlugin.h"
#include "catapult/keylink/KeyLinkObserver.h"
#include "catapult/keylink/KeyLinkValidator.h"
#include "catapult/keylink/MultiKeyLinkObserver.h"
#include "catapult/keylink/MultiKeyLinkValidator.h"
#include "catapult/model/BlockChainConfiguration.h"
#include "catapult/plugins/CacheHandlers.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace plugins {

	namespace {
		cache::AccountStateCacheTypes::Options CreateAccountStateCacheOptions(const model::BlockChainConfiguration& config) {
			return {
				config.Network.Identifier,
				config.ImportanceGrouping,
				config.VotingSetGrouping,
				config.MinHarvesterBalance,
				config.MaxHarvesterBalance,
				config.MinVoterBalance,
				config.CurrencyMosaicId,
				config.HarvestingMosaicId
			};
		}

		void AddAccountStateCache(PluginManager& manager, const model::BlockChainConfiguration& config) {
			using namespace catapult::cache;

			auto cacheConfig = manager.cacheConfig(AccountStateCache::Name);
			auto cacheOptions = CreateAccountStateCacheOptions(config);
			manager.addCacheSupport(std::make_unique<AccountStateCacheSubCachePlugin>(cacheConfig, cacheOptions));

			using CacheHandlers = CacheHandlers<cache::AccountStateCacheDescriptor>;
			CacheHandlers::Register<model::FacilityCode::Core>(manager);

			manager.addDiagnosticCounterHook([](auto& counters, const CatapultCache& cache) {
				counters.emplace_back(utils::DiagnosticCounterId("ACNTST C"), [&cache]() {
					return cache.sub<AccountStateCache>().createView()->size();
				});
				counters.emplace_back(utils::DiagnosticCounterId("ACNTST C HVA"), [&cache]() {
					return cache.sub<AccountStateCache>().createView()->highValueAccounts().addresses().size();
				});
			});
		}

		void AddBlockStatisticCache(PluginManager& manager, const model::BlockChainConfiguration& config) {
			using namespace catapult::cache;

			manager.addCacheSupport(std::make_unique<BlockStatisticCacheSubCachePlugin>(CalculateDifficultyHistorySize(config)));

			manager.addDiagnosticCounterHook([](auto& counters, const CatapultCache& cache) {
				counters.emplace_back(utils::DiagnosticCounterId("BLKDIF C"), [&cache]() {
					return cache.sub<BlockStatisticCache>().createView()->size();
				});
			});
		}

		struct BasicKeyAccessor {
			static constexpr auto Failure_Link_Already_Exists = validators::Failure_Core_Link_Already_Exists;
			static constexpr auto Failure_Inconsistent_Unlink_Data = validators::Failure_Core_Inconsistent_Unlink_Data;
			static constexpr auto Failure_Too_Many_Links = validators::Failure_Core_Too_Many_Links;
		};

		struct VrfKeyAccessor : public BasicKeyAccessor {
			template<typename TAccountState>
			static auto& Get(TAccountState& accountState) {
				return accountState.SupplementalPublicKeys.vrf();
			}
		};

		struct VotingKeyAccessor : public BasicKeyAccessor {
			template<typename TAccountState>
			static auto& Get(TAccountState& accountState) {
				return accountState.SupplementalPublicKeys.voting();
			}
		};

		void RegisterVrfKeyLinkTransaction(PluginManager& manager) {
			manager.addTransactionSupport(CreateVrfKeyLinkTransactionPlugin());

			manager.addStatefulValidatorHook([](auto& builder) {
				builder.add(keylink::CreateKeyLinkValidator<model::VrfKeyLinkNotification, VrfKeyAccessor>("Vrf"));
			});

			manager.addObserverHook([](auto& builder) {
				builder.add(keylink::CreateKeyLinkObserver<model::VrfKeyLinkNotification, VrfKeyAccessor>("Vrf"));
			});
		}

		void RegisterVotingKeyLinkTransaction(PluginManager& manager) {
			const auto& config = manager.config();

			manager.addTransactionSupport(CreateVotingKeyLinkTransactionPlugin());

			manager.addStatelessValidatorHook([&config](auto& builder) {
				builder.add(validators::CreateVotingKeyLinkRangeValidator(config.MinVotingKeyLifetime, config.MaxVotingKeyLifetime));
			});

			manager.addStatefulValidatorHook([&config](auto& builder) {
				builder.add(keylink::CreateMultiKeyLinkValidator<model::VotingKeyLinkNotification, VotingKeyAccessor>(
						"Voting",
						config.MaxVotingKeysPerAccount));
			});

			manager.addObserverHook([](auto& builder) {
				builder.add(keylink::CreateMultiKeyLinkObserver<model::VotingKeyLinkNotification, VotingKeyAccessor>("Voting"));
			});
		}
	}

	void RegisterCoreSystem(PluginManager& manager) {
		const auto& config = manager.config();

		AddAccountStateCache(manager, config);
		AddBlockStatisticCache(manager, config);

		manager.addStatelessValidatorHook([&config](auto& builder) {
			builder
				.add(validators::CreateZeroAddressValidator(config.Network.Identifier))
				.add(validators::CreateZeroPublicKeyValidator())
				.add(validators::CreateMaxTransactionsValidator(config.MaxTransactionsPerBlock))
				.add(validators::CreateNetworkValidator(config.Network.Identifier))
				.add(validators::CreateEntityVersionValidator())
				.add(validators::CreateTransactionFeeValidator())
				.add(validators::CreateKeyLinkActionValidator())
				.add(validators::CreateZeroInternalPaddingValidator());
		});

		manager.addStatefulValidatorHook([&config](auto& builder) {
			builder
				.add(validators::CreateAddressValidator())
				.add(validators::CreatePublicKeyValidator())
				.add(validators::CreateDeadlineValidator(config.MaxTransactionLifetime))
				.add(validators::CreateNemesisSinkValidator())
				.add(validators::CreateEligibleHarvesterValidator())
				.add(validators::CreateBalanceDebitValidator())
				.add(validators::CreateBalanceTransferValidator());
		});

		auto harvestFeeOptions = observers::HarvestFeeOptions{
			config.CurrencyMosaicId,
			config.HarvestBeneficiaryPercentage,
			config.HarvestNetworkPercentage,
			config.HarvestNetworkFeeSinkAddress
		};
		const auto& calculator = manager.inflationConfig().InflationCalculator;
		manager.addObserverHook([harvestFeeOptions, &calculator](auto& builder) {
			builder
				.add(observers::CreateSourceChangeObserver())
				.add(observers::CreateAccountAddressObserver())
				.add(observers::CreateAccountPublicKeyObserver())
				.add(observers::CreateBalanceDebitObserver())
				.add(observers::CreateBalanceTransferObserver())
				.add(observers::CreateBeneficiaryObserver())
				.add(observers::CreateTransactionFeeActivityObserver())
				.add(observers::CreateHarvestFeeObserver(harvestFeeOptions, calculator))
				.add(observers::CreateTotalTransactionsObserver())
				.add(observers::CreateHighValueAccountObserver(observers::NotifyMode::Commit));
		});

		manager.addTransientObserverHook([&config](auto& builder) {
			auto pRecalculateImportancesObserver = observers::CreateRecalculateImportancesObserver(
					importance::CreateImportanceCalculator(config),
					importance::CreateRestoreImportanceCalculator());
			builder
				.add(std::move(pRecalculateImportancesObserver))
				.add(observers::CreateHighValueAccountObserver(observers::NotifyMode::Rollback))
				.add(observers::CreateBlockStatisticObserver(config.MaxDifficultyBlocks, config.DefaultDynamicFeeMultiplier));
		});

		RegisterVrfKeyLinkTransaction(manager);
		RegisterVotingKeyLinkTransaction(manager);
	}
}}

extern "C" PLUGIN_API
void RegisterSubsystem(catapult::plugins::PluginManager& manager) {
	catapult::plugins::RegisterCoreSystem(manager);
}
