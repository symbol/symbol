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

#include "FinalizationBootstrapperService.h"
#include "finalization/src/chain/MultiRoundMessageAggregator.h"
#include "finalization/src/io/ProofStorageCache.h"
#include "finalization/src/model/FinalizationContext.h"
#include "catapult/cache/CatapultCache.h"
#include "catapult/cache_core/AccountStateCache.h"
#include "catapult/extensions/ConfigurationUtils.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/subscribers/FinalizationSubscriber.h"

namespace catapult { namespace finalization {

	namespace {
		constexpr auto Hooks_Service_Name = "fin.hooks";
		constexpr auto Storage_Service_Name = "fin.proof.storage";
		constexpr auto Aggregator_Service_Name = "fin.aggregator.multiround";

		// region FinalizationContextFactory

		class FinalizationContextFactory {
		public:
			FinalizationContextFactory(
					const FinalizationConfiguration& config,
					const cache::AccountStateCache& accountStateCache,
					const io::BlockStorageCache& storage,
					const io::ProofStorageCache& proofStorage)
					: m_config(config)
					, m_accountStateCache(accountStateCache)
					, m_storage(storage)
					, m_proofStorage(proofStorage)
			{}

		public:
			model::FinalizationContext create(FinalizationPoint roundPoint) const {
				// TODO: FinalizationContext construction, especially usage of storageContext, will need to change with voting sets
				auto storageContext = loadStorageContext();
				auto accountStateCacheView = m_accountStateCache.createView();
				return model::FinalizationContext(
						roundPoint,
						storageContext.LastFinalizedHeight,
						storageContext.LastFinalizedGenerationHash,
						m_config,
						*accountStateCacheView);
			}

		private:
			struct StorageContext {
				Height LastFinalizedHeight;
				GenerationHash LastFinalizedGenerationHash;
			};

		private:
			StorageContext loadStorageContext() const {
				auto proofStorageView = m_proofStorage.view();
				auto height = proofStorageView.finalizedHeight();

				auto generationHash = m_storage.view().loadBlockElement(height)->GenerationHash;
				return { height, generationHash };
			}

		private:
			FinalizationConfiguration m_config;
			const cache::AccountStateCache& m_accountStateCache;
			const io::BlockStorageCache& m_storage;
			const io::ProofStorageCache& m_proofStorage;
		};

		// endregion

		// region CreateMultiRoundMessageAggregator

		model::HeightHashPair LoadLastFinalizedHeightHashPair(const io::ProofStorageView& proofStorageView) {
			return *proofStorageView.loadFinalizedHashesFrom(proofStorageView.finalizationPoint() - FinalizationPoint(1), 1).cbegin();
		}

		auto CreateMultiRoundMessageAggregator(
				const FinalizationConfiguration& config,
				const io::ProofStorageCache& proofStorage,
				extensions::ServiceState& state) {
			auto maxResponseSize = config.MessageSynchronizationMaxResponseSize.bytes();
			FinalizationContextFactory finalizationContextFactory(
					config,
					state.cache().sub<cache::AccountStateCache>(),
					state.storage(),
					proofStorage);

			auto proofStorageView = proofStorage.view();
			return std::make_shared<chain::MultiRoundMessageAggregator>(
					maxResponseSize,
					proofStorageView.finalizationPoint(),
					LoadLastFinalizedHeightHashPair(proofStorageView),
					[maxResponseSize, finalizationContextFactory](auto roundPoint) {
						// TODO: will need update when we figure out voting sets o0
						return chain::CreateRoundMessageAggregator(maxResponseSize, finalizationContextFactory.create(roundPoint));
					});
		}

		// endregion

		// region FinalizationBootstrapperServiceRegistrar

		namespace {
			Height GetEstimateHeight(const chain::MultiRoundMessageAggregator& aggregator, FinalizationPoint delta) {
				auto view = aggregator.view();
				return view.findEstimate(view.maxFinalizationPoint() - delta).Height;
			}
		}

		class FinalizationBootstrapperServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			FinalizationBootstrapperServiceRegistrar(
					const FinalizationConfiguration& config,
					std::unique_ptr<io::ProofStorage>&& pProofStorage)
					: m_config(config)
					, m_pProofStorageCache(std::make_unique<io::ProofStorageCache>(std::move(pProofStorage)))
			{}

		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "FinalizationBootstrapper", extensions::ServiceRegistrarPhase::Initial };
			}

			void registerServiceCounters(extensions::ServiceLocator& locator) override {
				using AggregatorType = chain::MultiRoundMessageAggregator;

				locator.registerServiceCounter<AggregatorType>(Aggregator_Service_Name, "FIN MIN FP", [](const auto& aggregator) {
					return aggregator.view().minFinalizationPoint().unwrap();
				});
				locator.registerServiceCounter<AggregatorType>(Aggregator_Service_Name, "FIN MAX FP", [](const auto& aggregator) {
					return aggregator.view().maxFinalizationPoint().unwrap();
				});
				locator.registerServiceCounter<AggregatorType>(Aggregator_Service_Name, "FIN PREV EST", [](const auto& aggregator) {
					return GetEstimateHeight(aggregator, FinalizationPoint(1)).unwrap();
				});
				locator.registerServiceCounter<AggregatorType>(Aggregator_Service_Name, "FIN CUR EST", [](const auto& aggregator) {
					return GetEstimateHeight(aggregator, FinalizationPoint(0)).unwrap();
				});
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState& state) override {
				// register services
				locator.registerRootedService(Hooks_Service_Name, std::make_shared<FinalizationServerHooks>());

				locator.registerRootedService(Storage_Service_Name, m_pProofStorageCache);

				auto pMultiRoundMessageAggregator = CreateMultiRoundMessageAggregator(m_config, *m_pProofStorageCache, state);
				locator.registerRootedService(Aggregator_Service_Name, pMultiRoundMessageAggregator);
			}

		private:
			FinalizationConfiguration m_config;
			std::shared_ptr<io::ProofStorageCache> m_pProofStorageCache;
		};

		// endregion
	}

	DECLARE_SERVICE_REGISTRAR(FinalizationBootstrapper)(
			const FinalizationConfiguration& config,
			std::unique_ptr<io::ProofStorage>&& pProofStorage) {
		return std::make_unique<FinalizationBootstrapperServiceRegistrar>(config, std::move(pProofStorage));
	}

	chain::MultiRoundMessageAggregator& GetMultiRoundMessageAggregator(const extensions::ServiceLocator& locator) {
		return *locator.service<chain::MultiRoundMessageAggregator>(Aggregator_Service_Name);
	}

	FinalizationServerHooks& GetFinalizationServerHooks(const extensions::ServiceLocator& locator) {
		return *locator.service<FinalizationServerHooks>(Hooks_Service_Name);
	}

	io::ProofStorageCache& GetProofStorageCache(const extensions::ServiceLocator& locator) {
		return *locator.service<io::ProofStorageCache>(Storage_Service_Name);
	}
}}
