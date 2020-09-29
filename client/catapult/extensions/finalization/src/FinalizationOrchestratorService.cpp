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
#include "FinalizationContextFactory.h"
#include "VotingStatusFile.h"
#include "finalization/src/chain/MultiRoundMessageAggregator.h"
#include "finalization/src/io/ProofStorageCache.h"
#include "finalization/src/model/VotingSet.h"
#include "catapult/config/CatapultDataDirectory.h"
#include "catapult/crypto_voting/AggregateBmPrivateKeyTree.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/io/BlockStorageCache.h"
#include "catapult/io/FileStream.h"

namespace catapult { namespace finalization {

	namespace {
		constexpr auto Orchestrator_Service_Name = "fin.orchestrator";

		// region BootstrapperFacade

		class BootstrapperFacade {
		private:
			enum class EpochStatus { Continue, Wait, Advance };

		public:
			BootstrapperFacade(
					const FinalizationConfiguration& config,
					const extensions::ServiceLocator& locator,
					extensions::ServiceState& state)
					: m_votingSetGrouping(config.VotingSetGrouping)
					, m_messageAggregator(GetMultiRoundMessageAggregator(locator))
					, m_hooks(GetFinalizationServerHooks(locator))
					, m_proofStorage(GetProofStorageCache(locator))
					, m_blockStorage(state.storage())
					, m_votingStatusFile(config::CatapultDirectory(state.config().User.DataDirectory).file("voting_status.dat"))
					, m_orchestrator(
							config.EnableRevoteOnBoot ? LoadVotingStatusFromStorage(m_proofStorage) : m_votingStatusFile.load(),
							[stepDuration = config.StepDuration, &messageAggregator = m_messageAggregator](auto point, auto time) {
								return chain::CreateFinalizationStageAdvancer(point, time, stepDuration, messageAggregator);
							},
							[factory = FinalizationContextFactory(config, state)](const auto& message) {
								const auto& votingPublicKey = message.Signature.Root.ParentPublicKey;
								return factory.create(message.StepIdentifier.Epoch).isEligibleVoter(votingPublicKey);
							},
							[&hooks = m_hooks](auto&& pMessage) {
								hooks.messageRangeConsumer()(model::FinalizationMessageRange::FromEntity(std::move(pMessage)));
							},
							chain::CreateFinalizationMessageFactory(
									config,
									state.storage(),
									m_proofStorage,
									CreateVotingPrivateKeyTree(state.config().User)))
					, m_finalizer(CreateFinalizer(m_messageAggregator, m_proofStorage))
			{}

		public:
			void poll(Timestamp time) {
				auto orchestratorRound = m_orchestrator.votingStatus().Round;

				auto epochStatusResult = calculateEpochStatus(orchestratorRound.Epoch);
				if (EpochStatus::Wait == epochStatusResult.first)
					return;

				if (EpochStatus::Advance == epochStatusResult.first) {
					m_orchestrator.setEpoch(epochStatusResult.second);
					orchestratorRound = m_orchestrator.votingStatus().Round;

					CATAPULT_LOG(debug) << "advancing to epoch " << orchestratorRound;
				}

				if (orchestratorRound > m_messageAggregator.view().maxFinalizationRound())
					m_messageAggregator.modifier().setMaxFinalizationRound(orchestratorRound);

				m_orchestrator.poll(time);
				m_votingStatusFile.save(m_orchestrator.votingStatus());
				m_finalizer();
			}

		private:
			std::pair<EpochStatus, FinalizationEpoch> calculateEpochStatus(FinalizationEpoch epoch) const {
				auto finalizationStatistics = m_proofStorage.view().statistics();

				auto isStorageEpochAhead = finalizationStatistics.Round.Epoch > epoch;
				if (isStorageEpochAhead) {
					CATAPULT_LOG(info)
							<< "proof storage epoch " << finalizationStatistics.Round.Epoch
							<< " is out of sync with current epoch " << epoch;
				}

				auto votingSetEndHeight = model::CalculateVotingSetEndHeight(epoch, m_votingSetGrouping);
				if (!isStorageEpochAhead && finalizationStatistics.Height != votingSetEndHeight)
					return std::make_pair(EpochStatus::Continue, FinalizationEpoch());

				auto blockStorageView = m_blockStorage.view();
				auto localChainHeight = blockStorageView.chainHeight();
				if (localChainHeight < finalizationStatistics.Height) {
					CATAPULT_LOG(warning)
							<< "waiting for sync before transitioning from epoch " << epoch
							<< " (height " << localChainHeight
							<< " < finalized height " << finalizationStatistics.Height << ")";
					return std::make_pair(EpochStatus::Wait, FinalizationEpoch());
				}

				auto localBlockHash = *blockStorageView.loadHashesFrom(finalizationStatistics.Height, 1).cbegin();
				if (localBlockHash != finalizationStatistics.Hash) {
					CATAPULT_LOG(warning)
							<< "waiting for sync before transitioning from epoch " << epoch
							<< " (hash " << localBlockHash
							<< " != finalized hash " << finalizationStatistics.Hash << ")";
					return std::make_pair(EpochStatus::Wait, FinalizationEpoch());
				}

				auto newEpoch = finalizationStatistics.Round.Epoch + FinalizationEpoch(1);
				return std::make_pair(EpochStatus::Advance, newEpoch);
			}

		private:
			chain::VotingStatus LoadVotingStatusFromStorage(const io::ProofStorageCache& proofStorage) {
				auto storageRound = proofStorage.view().statistics().Round;
				return { { storageRound.Epoch, storageRound.Point + FinalizationPoint(1) } };
			}

			static crypto::AggregateBmPrivateKeyTree CreateVotingPrivateKeyTree(const config::UserConfiguration& userConfig) {
				auto factory = CreateBmPrivateKeyTreeFactory(config::CatapultDirectory(userConfig.VotingKeysDirectory));
				return crypto::AggregateBmPrivateKeyTree(factory);
			}

			static std::string GetVotingPrivateKeyTreeFilename(uint64_t treeSequenceId) {
				std::ostringstream out;
				out << "private_key_tree" << treeSequenceId << ".dat";
				return out.str();
			}

			static supplier<std::unique_ptr<crypto::BmPrivateKeyTree>> CreateBmPrivateKeyTreeFactory(
					const config::CatapultDirectory& directory) {
				auto treeSequenceId = 1u;
				std::shared_ptr<io::FileStream> pKeyTreeStream;
				return [treeSequenceId, pKeyTreeStream, directory]() mutable {
					auto keyTreeFilename = directory.file(GetVotingPrivateKeyTreeFilename(treeSequenceId++));
					CATAPULT_LOG(debug) << "loading voting private key tree from " << keyTreeFilename;

					auto keyTreeFile = io::RawFile(keyTreeFilename, io::OpenMode::Read_Append);
					pKeyTreeStream = std::make_shared<io::FileStream>(std::move(keyTreeFile));

					auto tree = crypto::BmPrivateKeyTree::FromStream(*pKeyTreeStream);
					return std::make_unique<crypto::BmPrivateKeyTree>(std::move(tree));
				};
			}

		private:
			uint64_t m_votingSetGrouping;
			chain::MultiRoundMessageAggregator& m_messageAggregator;
			FinalizationServerHooks& m_hooks;
			io::ProofStorageCache& m_proofStorage;
			io::BlockStorageCache& m_blockStorage;

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
