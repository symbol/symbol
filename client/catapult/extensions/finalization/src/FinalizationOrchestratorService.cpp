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

#include "FinalizationOrchestratorService.h"
#include "FinalizationBootstrapperService.h"
#include "FinalizationConfiguration.h"
#include "VotingStatusFile.h"
#include "finalization/src/chain/MultiRoundMessageAggregator.h"
#include "finalization/src/io/ProofStorageCache.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/crypto_voting/OtsTree.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/io/FileStream.h"

namespace catapult { namespace finalization {

	namespace {
		constexpr auto Orchestrator_Service_Name = "fin.orchestrator";

		// region BootstrapperFacade

		class BootstrapperFacade {
		private:
			static constexpr auto LoadOtsTree = crypto::OtsTree::FromStream;

		public:
			BootstrapperFacade(
					const FinalizationConfiguration& config,
					const extensions::ServiceLocator& locator,
					extensions::ServiceState& state)
					: m_messageAggregator(GetMultiRoundMessageAggregator(locator))
					, m_hooks(GetFinalizationServerHooks(locator))
					, m_proofStorage(GetProofStorageCache(locator))
					, m_otsStream(io::RawFile(QualifyFilename(state, "voting_ots_tree.dat"), io::OpenMode::Read_Append))
					, m_votingStatusFile(QualifyFilename(state, "voting_status.dat"))
					, m_orchestrator(
							m_votingStatusFile.load(),
							[stepDuration = config.StepDuration, &messageAggregator = m_messageAggregator](auto point, auto time) {
								return chain::CreateFinalizationStageAdvancer(point, time, stepDuration, messageAggregator);
							},
							[&hooks = m_hooks](auto&& pMessage) {
								hooks.messageRangeConsumer()(model::FinalizationMessageRange::FromEntity(std::move(pMessage)));
							},
							chain::CreateFinalizationMessageFactory(config, state.storage(), m_proofStorage, LoadOtsTree(m_otsStream)))
					, m_finalizer(CreateFinalizer(m_messageAggregator, state.finalizationSubscriber(), m_proofStorage))
			{}

		public:
			void poll(Timestamp time) {
				if (m_orchestrator.point() > m_messageAggregator.view().maxFinalizationPoint())
					m_messageAggregator.modifier().setMaxFinalizationPoint(m_orchestrator.point());

				m_orchestrator.poll(time);
				m_votingStatusFile.save({ m_orchestrator.point(), m_orchestrator.hasSentPrevote(), m_orchestrator.hasSentPrecommit() });
				m_finalizer();
			}

		private:
			static std::string QualifyFilename(const extensions::ServiceState& state, const std::string& name) {
				return config::CatapultDataDirectory(state.config().User.DataDirectory).rootDir().file(name);
			}

		private:
			chain::MultiRoundMessageAggregator& m_messageAggregator;
			FinalizationServerHooks& m_hooks;
			io::ProofStorageCache& m_proofStorage;

			io::FileStream m_otsStream;
			VotingStatusFile m_votingStatusFile;
			chain::FinalizationOrchestrator m_orchestrator;
			action m_finalizer;
		};

		// endregion

		// region FinalizationOrchestratorServiceRegistrar

		thread::Task CreateFinalizationTask(BootstrapperFacade& facade, const supplier<Timestamp>& timeSupplier) {
			thread::Task task;
			task.Name = "finalization task";
			task.Callback = [&facade, timeSupplier]() {
				facade.poll(timeSupplier());
				return thread::make_ready_future(thread::TaskResult::Continue);
			};
			return task;
		}

		class FinalizationOrchestratorServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			explicit FinalizationOrchestratorServiceRegistrar(const FinalizationConfiguration& config) : m_config(config)
			{}

		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "FinalizationOrchestrator", extensions::ServiceRegistrarPhase::Post_Extended_Range_Consumers };
			}

			void registerServiceCounters(extensions::ServiceLocator&) override {
				// no counters
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState& state) override {
				// register services
				auto pOrchestrator = std::make_shared<BootstrapperFacade>(m_config, locator, state);
				locator.registerRootedService(Orchestrator_Service_Name, pOrchestrator);

				// add task
				state.tasks().push_back(CreateFinalizationTask(*pOrchestrator, state.timeSupplier()));
			}

		private:
			FinalizationConfiguration m_config;
		};

		// endregion
	}

	DECLARE_SERVICE_REGISTRAR(FinalizationOrchestrator)(const FinalizationConfiguration& config) {
		return std::make_unique<FinalizationOrchestratorServiceRegistrar>(config);
	}
}}
