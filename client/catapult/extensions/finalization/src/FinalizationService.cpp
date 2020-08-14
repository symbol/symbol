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

#include "FinalizationService.h"
#include "FinalizationBootstrapperService.h"
#include "finalization/src/api/RemoteFinalizationApi.h"
#include "finalization/src/chain/FinalizationMessageSynchronizer.h"
#include "finalization/src/chain/MultiRoundMessageAggregator.h"
#include "catapult/config/CatapultKeys.h"
#include "catapult/extensions/NetworkUtils.h"
#include "catapult/extensions/PeersConnectionTasks.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/extensions/ServiceUtils.h"
#include "catapult/extensions/SynchronizerTaskCallbacks.h"
#include "catapult/net/PacketWriters.h"
#include "catapult/thread/MultiServicePool.h"

namespace catapult { namespace finalization {

	namespace {
		constexpr auto Service_Name = "fin.writers";
		constexpr auto Service_Id = ionet::ServiceIdentifier(0x50415254);

		thread::Task CreateConnectPeersTask(extensions::ServiceState& state, net::PacketWriters& packetWriters) {
			auto settings = extensions::SelectorSettings(
					state.cache(),
					state.config().BlockChain.TotalChainImportance,
					state.nodes(),
					Service_Id,
					ionet::NodeRoles::Voting,
					state.config().Node.OutgoingConnections);
			auto task = extensions::CreateConnectPeersTask(settings, packetWriters);
			task.Name += " for service Finalization";
			return task;
		}

		// shim to allow RemoteFinalizationApi to be used with CreateChainSyncAwareSynchronizerTaskCallback
		std::unique_ptr<api::RemoteFinalizationApi> CreateRemoteFinalizationApi(
				ionet::PacketIo& io,
				const model::NodeIdentity& remoteIdentity,
				const model::TransactionRegistry&) {
			return api::CreateRemoteFinalizationApi(io, remoteIdentity);
		}

		thread::Task CreatePullFinalizationTask(
				extensions::ServiceLocator& locator,
				const extensions::ServiceState& state,
				net::PacketWriters& packetWriters) {
			const auto& messageAggregator = GetMultiRoundMessageAggregator(locator);
			const auto& serverHooks = GetFinalizationServerHooks(locator);
			auto finalizationMessageSynchronizer = chain::CreateFinalizationMessageSynchronizer(
					[&messageAggregator]() {
						auto messageAggregatorView = messageAggregator.view();
						return std::make_pair(messageAggregatorView.maxFinalizationPoint(), messageAggregatorView.shortHashes());
					},
					serverHooks.messageRangeConsumer());

			thread::Task task;
			task.Name = "pull finalization messages task";
			task.Callback = CreateChainSyncAwareSynchronizerTaskCallback(
					std::move(finalizationMessageSynchronizer),
					CreateRemoteFinalizationApi,
					packetWriters,
					state,
					task.Name);
			return task;
		}

		class FinalizationServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "Finalization", extensions::ServiceRegistrarPhase::Post_Extended_Range_Consumers };
			}

			void registerServiceCounters(extensions::ServiceLocator& locator) override {
				locator.registerServiceCounter<net::PacketWriters>(Service_Name, "FIN WRITERS", [](const auto& writers) {
					return writers.numActiveWriters();
				});
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState& state) override {
				auto connectionSettings = extensions::GetConnectionSettings(state.config());
				auto pServiceGroup = state.pool().pushServiceGroup("finalization");
				auto pWriters = pServiceGroup->pushService(net::CreatePacketWriters, locator.keys().caPublicKey(), connectionSettings);

				locator.registerService(Service_Name, pWriters);
				state.packetIoPickers().insert(*pWriters, ionet::NodeRoles::Voting);

				// add sinks
				state.hooks().addBannedNodeIdentitySink(extensions::CreateCloseConnectionSink(*pWriters));

				// add tasks
				state.tasks().push_back(CreateConnectPeersTask(state, *pWriters));
				state.tasks().push_back(CreatePullFinalizationTask(locator, state, *pWriters));
			}
		};
	}

	DECLARE_SERVICE_REGISTRAR(Finalization)() {
		return std::make_unique<FinalizationServiceRegistrar>();
	}
}}
