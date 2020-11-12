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
#include "finalization/src/chain/FinalizationPatchingSubscriber.h"
#include "finalization/src/chain/MultiRoundMessageAggregator.h"
#include "finalization/src/io/AggregateProofStorage.h"
#include "finalization/src/io/FilePrevoteChainStorage.h"
#include "finalization/src/io/ProofStorageCache.h"
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
		constexpr auto Subscriber_Adapter_Service_Name = "fin.subscriber.adapter";

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
					finalizationStatistics.Round,
					model::HeightHashPair{ finalizationStatistics.Height, finalizationStatistics.Hash },
					[finalizationContextFactory](const auto& round) {
						return chain::CreateRoundMessageAggregator(finalizationContextFactory.create(round.Epoch));
					});
		}

		// endregion

		// region FinalizationSubscriberAdapter / CreateProofStorageCache

		class FinalizationSubscriberAdapter : public subscribers::FinalizationSubscriber {
		public:
			explicit FinalizationSubscriberAdapter(subscribers::FinalizationSubscriber& subscriber) : m_subscriber(subscriber)
			{}

		public:
			void setPatchingSubscriber(extensions::ServiceState& state) {
				// create patching subscriber (only Remote_Pull allows deep rollbacks)
				m_pPrevoteChainStorage = std::make_unique<io::FilePrevoteChainStorage>(state.config().User.DataDirectory);
				m_pPatchingSubscriber = std::make_unique<chain::FinalizationPatchingSubscriber>(
						*m_pPrevoteChainStorage,
						state.storage(),
						state.hooks().blockRangeConsumerFactory()(disruptor::InputSource::Remote_Pull));
			}

		public:
			void notifyFinalizedBlock(const model::FinalizationRound& round, Height height, const Hash256& hash) override {
				m_subscriber.notifyFinalizedBlock(round, height, hash);

				if (m_pPatchingSubscriber)
					m_pPatchingSubscriber->notifyFinalizedBlock(round, height, hash);
			}

		private:
			subscribers::FinalizationSubscriber& m_subscriber;
			std::unique_ptr<io::PrevoteChainStorage> m_pPrevoteChainStorage;
			std::unique_ptr<chain::FinalizationPatchingSubscriber> m_pPatchingSubscriber;
		};

		std::shared_ptr<io::ProofStorageCache> CreateProofStorageCache(
				std::unique_ptr<io::ProofStorage>&& pProofStorage,
				extensions::ServiceLocator& locator,
				extensions::ServiceState& state) {
			// create subscriber adapter (registration is only for phase two access, not ownership)
			auto pSubscriber = std::make_unique<FinalizationSubscriberAdapter>(state.finalizationSubscriber());
			locator.registerRootedService(
					Subscriber_Adapter_Service_Name,
					std::shared_ptr<FinalizationSubscriberAdapter>(pSubscriber.get(), [](const auto*) {}));

			// create proof storage cache
			return std::make_shared<io::ProofStorageCache>(io::CreateAggregateProofStorage(
					std::move(pProofStorage),
					std::move(pSubscriber)));
		}

		// endregion

		// region FinalizationBootstrapperServiceRegistrar

		namespace {
			Height GetEstimateHeight(const chain::MultiRoundMessageAggregator& aggregator, FinalizationPoint delta) {
				auto view = aggregator.view();
				return view.findEstimate(view.maxFinalizationRound() - delta).Height;
			}

			Hash256 LoadHashAtHeight(const io::BlockStorageView& blockStorage, Height height) {
				if (height > blockStorage.chainHeight())
					return Hash256();

				auto storageHashRange = blockStorage.loadHashesFrom(height, 1);
				return *storageHashRange.cbegin();
			}
		}

		class FinalizationBootstrapperServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			FinalizationBootstrapperServiceRegistrar(
					const FinalizationConfiguration& config,
					std::unique_ptr<io::ProofStorage>&& pProofStorage)
					: m_config(config)
					, m_pProofStorage(std::move(pProofStorage))
			{}

		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "FinalizationBootstrapper", extensions::ServiceRegistrarPhase::Initial };
			}

			void registerServiceCounters(extensions::ServiceLocator& locator) override {
				using AggregatorType = chain::MultiRoundMessageAggregator;

				locator.registerServiceCounter<AggregatorType>(Aggregator_Service_Name, "FIN MIN EPOCH", [](const auto& aggregator) {
					return aggregator.view().minFinalizationRound().Epoch.unwrap();
				});
				locator.registerServiceCounter<AggregatorType>(Aggregator_Service_Name, "FIN MIN POINT", [](const auto& aggregator) {
					return aggregator.view().minFinalizationRound().Point.unwrap();
				});
				locator.registerServiceCounter<AggregatorType>(Aggregator_Service_Name, "FIN MAX EPOCH", [](const auto& aggregator) {
					return aggregator.view().maxFinalizationRound().Epoch.unwrap();
				});
				locator.registerServiceCounter<AggregatorType>(Aggregator_Service_Name, "FIN MAX POINT", [](const auto& aggregator) {
					return aggregator.view().maxFinalizationRound().Point.unwrap();
				});
				locator.registerServiceCounter<AggregatorType>(Aggregator_Service_Name, "FIN PREV EST", [](const auto& aggregator) {
					return GetEstimateHeight(aggregator, FinalizationPoint(1)).unwrap();
				});
				locator.registerServiceCounter<AggregatorType>(Aggregator_Service_Name, "FIN CUR EST", [](const auto& aggregator) {
					return GetEstimateHeight(aggregator, FinalizationPoint(0)).unwrap();
				});
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState& state) override {
				// create proof storage cache
				auto pProofStorageCache = CreateProofStorageCache(std::move(m_pProofStorage), locator, state);

				// register hooks
				const auto& blockStorage = state.storage();
				state.hooks().setLocalFinalizedHeightHashPairSupplier([&proofStorage = *pProofStorageCache, &blockStorage]() {
					// 1. check the most recent proof for a match in storage
					auto finalizationStatistics = proofStorage.view().statistics();
					if (finalizationStatistics.Hash == LoadHashAtHeight(blockStorage.view(), finalizationStatistics.Height))
						return model::HeightHashPair{ finalizationStatistics.Height, finalizationStatistics.Hash };

					// 2. reverse search through epochs
					auto epoch = finalizationStatistics.Round.Epoch;
					while (true) {
						epoch = epoch - FinalizationEpoch(1);

						auto pProof = proofStorage.view().loadProof(epoch);
						if (pProof->Hash == LoadHashAtHeight(blockStorage.view(), pProof->Height))
							return model::HeightHashPair{ pProof->Height, pProof->Hash };
					}
				});
				state.hooks().setNetworkFinalizedHeightHashPairSupplier([&proofStorage = *pProofStorageCache]() {
					auto finalizationStatistics = proofStorage.view().statistics();
					return model::HeightHashPair{ finalizationStatistics.Height, finalizationStatistics.Hash };
				});

				// register services
				locator.registerRootedService(Hooks_Service_Name, std::make_shared<FinalizationServerHooks>());

				locator.registerRootedService(Storage_Service_Name, pProofStorageCache);

				auto pMultiRoundMessageAggregator = CreateMultiRoundMessageAggregator(m_config, *pProofStorageCache, state);
				locator.registerRootedService(Aggregator_Service_Name, pMultiRoundMessageAggregator);
			}

		private:
			FinalizationConfiguration m_config;
			std::unique_ptr<io::ProofStorage> m_pProofStorage;
		};

		// endregion

		// region FinalizationBootstrapperPhaseTwoServiceRegistrar

		class FinalizationBootstrapperPhaseTwoServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "FinalizationBootstrapperPhaseTwo", extensions::ServiceRegistrarPhase::Post_Range_Consumers };
			}

			void registerServiceCounters(extensions::ServiceLocator&) override {
				// no counters
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState& state) override {
				// set patching subscriber
				auto& subscriber = *locator.service<FinalizationSubscriberAdapter>(Subscriber_Adapter_Service_Name);
				subscriber.setPatchingSubscriber(state);
			}
		};

		// endregion
	}

	DECLARE_SERVICE_REGISTRAR(FinalizationBootstrapper)(
			const FinalizationConfiguration& config,
			std::unique_ptr<io::ProofStorage>&& pProofStorage) {
		return std::make_unique<FinalizationBootstrapperServiceRegistrar>(config, std::move(pProofStorage));
	}

	DECLARE_SERVICE_REGISTRAR(FinalizationBootstrapperPhaseTwo)() {
		return std::make_unique<FinalizationBootstrapperPhaseTwoServiceRegistrar>();
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
