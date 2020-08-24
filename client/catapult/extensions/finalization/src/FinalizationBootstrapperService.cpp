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
#include "FinalizationContextFactory.h"
#include "finalization/src/chain/MultiRoundMessageAggregator.h"
#include "finalization/src/io/ProofStorageCache.h"
#include "catapult/extensions/ConfigurationUtils.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/subscribers/FinalizationSubscriber.h"

namespace catapult { namespace finalization {

	namespace {
		constexpr auto Hooks_Service_Name = "fin.hooks";
		constexpr auto Storage_Service_Name = "fin.proof.storage";
		constexpr auto Aggregator_Service_Name = "fin.aggregator.multiround";

		// region CreateMultiRoundMessageAggregator

		auto CreateMultiRoundMessageAggregator(
				const FinalizationConfiguration& config,
				const io::ProofStorageCache& proofStorage,
				extensions::ServiceState& state) {
			FinalizationContextFactory finalizationContextFactory(config, state);

			auto proofStorageView = proofStorage.view();
			auto finalizationStatistics = proofStorageView.statistics();
			return std::make_shared<chain::MultiRoundMessageAggregator>(
					config.MessageSynchronizationMaxResponseSize.bytes(),
					finalizationStatistics.Point,
					model::HeightHashPair{ finalizationStatistics.Height, finalizationStatistics.Hash },
					[finalizationContextFactory](auto roundPoint, auto height) {
						return chain::CreateRoundMessageAggregator(finalizationContextFactory.create(roundPoint, height));
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
				// register hooks
				state.hooks().setLocalFinalizedHeightSupplier([&proofStorage = *m_pProofStorageCache]() {
					return proofStorage.view().statistics().Height;
				});

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
