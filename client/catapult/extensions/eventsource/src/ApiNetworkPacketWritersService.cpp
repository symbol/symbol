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

#include "ApiNetworkPacketWritersService.h"
#include "catapult/config/CatapultKeys.h"
#include "catapult/extensions/NetworkUtils.h"
#include "catapult/extensions/PeersConnectionTasks.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/extensions/ServiceUtils.h"
#include "catapult/ionet/BroadcastUtils.h"
#include "catapult/thread/MultiServicePool.h"

namespace catapult { namespace eventsource {

	namespace {
		constexpr auto Service_Name = "api.writers";
		constexpr auto Service_Id = ionet::ServiceIdentifier(0x41504957);

		using BlockSink = extensions::NewBlockSink;
		using TransactionsSink = extensions::SharedNewTransactionsSink;

		thread::Task CreateAgePeersTask(extensions::ServiceState& state, net::ConnectionContainer& connectionContainer) {
			auto settings = extensions::SelectorSettings(
					state.cache(),
					state.config().BlockChain.TotalChainImportance,
					state.nodes(),
					Service_Id,
					state.config().Node.IncomingConnections);
			auto task = extensions::CreateAgePeersTask(settings, connectionContainer);
			task.Name += " for service Api Writers";
			return task;
		}

		class ApiNetworkPacketWritersServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "ApiNetworkPacketWriters", extensions::ServiceRegistrarPhase::Initial };
			}

			void registerServiceCounters(extensions::ServiceLocator& locator) override {
				locator.registerServiceCounter<net::PacketWriters>(Service_Name, "B WRITERS", [](const auto& writers) {
					return writers.numActiveWriters();
				});
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState& state) override {
				const auto& config = state.config();
				auto connectionSettings = extensions::GetConnectionSettings(config);
				auto pServiceGroup = state.pool().pushServiceGroup("api");
				auto pWriters = pServiceGroup->pushService(net::CreatePacketWriters, locator.keys().caPublicKey(), connectionSettings);
				extensions::BootServer(
						*pServiceGroup,
						config.Node.ApiPort,
						Service_Id,
						config,
						state.timeSupplier(),
						state.nodeSubscriber(),
						*pWriters);

				locator.registerService(Service_Name, pWriters);

				// add sinks
				state.hooks().addNewBlockSink(extensions::CreatePushEntitySink<BlockSink>(locator, Service_Name));
				state.hooks().addNewTransactionsSink(extensions::CreatePushEntitySink<TransactionsSink>(locator, Service_Name));
				state.hooks().addBannedNodeIdentitySink(extensions::CreateCloseConnectionSink(*pWriters));

				// add tasks
				state.tasks().push_back(CreateAgePeersTask(state, *pWriters));
			}
		};
	}

	DECLARE_SERVICE_REGISTRAR(ApiNetworkPacketWriters)() {
		return std::make_unique<ApiNetworkPacketWritersServiceRegistrar>();
	}
}}
