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
#include "finalization/src/chain/FinalizationOrchestrator.h"
#include "finalization/src/chain/MultiRoundMessageAggregator.h"
#include "finalization/src/io/ProofStorageCache.h"
#include "catapult/crypto_voting/OtsTree.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/extensions/ServiceState.h"

namespace catapult { namespace finalization {

	namespace {
		constexpr auto Orchestrator_Service_Name = "fin.orchestrator";

		// region SeekableMemoryStream

		class SeekableMemoryStream : public io::SeekableStream {
		public:
			SeekableMemoryStream() : m_position(0)
			{}

		public:
			void write(const RawBuffer& buffer) {
				m_buffer.resize(std::max<size_t>(m_buffer.size(), m_position + buffer.Size));
				std::memcpy(&m_buffer[m_position], buffer.pData, buffer.Size);
				m_position += buffer.Size;
			}

			void flush()
			{}

			void seek(uint64_t position) {
				m_position = position;
			}

			uint64_t position() const {
				return m_position;
			}

			bool eof() const {
				return m_position == m_buffer.size();
			}

			void read(const MutableRawBuffer& buffer) {
				if (m_position + buffer.Size > m_buffer.size())
					CATAPULT_THROW_RUNTIME_ERROR("invalid read()");

				std::memcpy(buffer.pData, &m_buffer[m_position], buffer.Size);
				m_position += buffer.Size;
			}

		private:
			std::vector<uint8_t> m_buffer;
			uint64_t m_position;
		};

		// endregion

		// region BootstrapperFacade

		class BootstrapperFacade {
		public:
			BootstrapperFacade(
					const FinalizationConfiguration& config,
					const extensions::ServiceLocator& locator,
					extensions::ServiceState& state)
					: m_messageAggregator(GetMultiRoundMessageAggregator(locator))
					, m_hooks(GetFinalizationServerHooks(locator))
					, m_proofStorage(GetProofStorageCache(locator))
					, m_orchestrator(
							m_proofStorage.view().finalizationPoint() + FinalizationPoint(1),
							[stepDuration = config.StepDuration, &messageAggregator = m_messageAggregator](auto point, auto time) {
								return chain::CreateFinalizationStageAdvancer(point, time, stepDuration, messageAggregator);
							},
							[&hooks = m_hooks](auto&& pMessage) {
								hooks.messageRangeConsumer()(model::FinalizationMessageRange::FromEntity(std::move(pMessage)));
							},
							chain::CreateFinalizationMessageFactory(config, state.storage(), m_proofStorage, CreateOtsTree(m_otsStream)))
					, m_finalizer(CreateFinalizer(m_messageAggregator, state.finalizationSubscriber(), m_proofStorage))
			{}

		public:
			void poll(Timestamp time) {
				m_orchestrator.poll(time);

				if (m_orchestrator.point() > m_messageAggregator.view().maxFinalizationPoint())
					m_messageAggregator.modifier().setMaxFinalizationPoint(m_orchestrator.point());

				m_finalizer();
			}

		private:
			// TODO: gimre - how do you imagine this working?
			static crypto::OtsTree CreateOtsTree(SeekableMemoryStream& storage) {
				auto dilution = 13u;
				auto startKeyIdentifier = model::StepIdentifierToOtsKeyIdentifier({ 1, 0, 0 }, dilution);
				auto endKeyIdentifier = model::StepIdentifierToOtsKeyIdentifier({ 100, 1, 0 }, dilution);

				return crypto::OtsTree::Create(
						crypto::KeyPair::FromString("934B1829665F7324362380E844CBEDA2C103AAEFD3A2C4645DC1715AC29E52E6"),
						storage,
						{ dilution, startKeyIdentifier, endKeyIdentifier });
			}

		private:
			chain::MultiRoundMessageAggregator& m_messageAggregator;
			FinalizationServerHooks& m_hooks;
			io::ProofStorageCache& m_proofStorage;

			SeekableMemoryStream m_otsStream;
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
