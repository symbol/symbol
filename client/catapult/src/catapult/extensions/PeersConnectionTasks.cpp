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

#include "PeersConnectionTasks.h"
#include "catapult/net/PacketWriters.h"
#include "catapult/thread/FutureUtils.h"
#include "catapult/utils/Logging.h"

namespace catapult { namespace extensions {

	// region CreateNodeAger

	NodeAger CreateNodeAger(
			ionet::ServiceIdentifier serviceId,
			const config::NodeConfiguration::ConnectionsSubConfiguration& config,
			ionet::NodeContainer& nodes) {
		return [&nodes, serviceId, config](const auto& identities) {
			// age all connections
			auto modifier = nodes.modifier();
			modifier.ageConnections(serviceId, identities);
			modifier.ageConnectionBans(serviceId, config.MaxConnectionBanAge, config.NumConsecutiveFailuresBeforeBanning);
		};
	}

	// endregion

	// region CreateNodeSelector / CreateConnectPeersTask

	namespace {
		CPP14_CONSTEXPR
		utils::LogLevel MapToLogLevel(net::PeerConnectCode connectCode) {
			switch (connectCode) {
			case net::PeerConnectCode::Accepted:
				return utils::LogLevel::Info;

			default:
				return utils::LogLevel::Warning;
			}
		}

		class AddCandidateProcessor {
		private:
			using ConnectResult = std::pair<Key, net::PeerConnectCode>;
			using ConnectResultsFuture = thread::future<std::vector<thread::future<ConnectResult>>>;

		public:
			struct AddState {
				ionet::NodeContainer& Nodes;
				net::PacketWriters& PacketWriters;
				ionet::ServiceIdentifier ServiceId;
			};

		public:
			AddCandidateProcessor(const AddState& state) : m_state(state)
			{}

		public:
			thread::future<thread::TaskResult> process(const ionet::NodeSet& addCandidates) {
				if (addCandidates.empty()) {
					CATAPULT_LOG(debug) << "no add candidates for service " << m_state.ServiceId;
					return thread::make_ready_future(thread::TaskResult::Continue);
				}

				auto connectFutures = createConnectFutures(addCandidates);
				return thread::when_all(std::move(connectFutures)).then([state = m_state](auto&& connectResultsFuture) {
					// update interaction information for all nodes
					UpdateInteractions(state, std::move(connectResultsFuture));
					return thread::TaskResult::Continue;
				});
			}

		private:
			std::vector<thread::future<ConnectResult>> createConnectFutures(const ionet::NodeSet& addCandidates) {
				auto i = 0u;
				std::vector<thread::future<ConnectResult>> futures(addCandidates.size());
				for (const auto& node : addCandidates) {
					auto pPromise = std::make_shared<thread::promise<ConnectResult>>();
					futures[i++] = pPromise->get_future();

					m_state.PacketWriters.connect(node, [node, pPromise](const auto& connectResult) {
						CATAPULT_LOG_LEVEL(MapToLogLevel(connectResult.Code))
								<< "connection attempt to " << node << " completed with " << connectResult.Code;
						pPromise->set_value(std::make_pair(node.identityKey(), connectResult.Code));
					});
				}

				return futures;
			}

		private:
			static void UpdateInteractions(const AddState& state, ConnectResultsFuture&& connectResultsFuture) {
				auto modifier = state.Nodes.modifier();
				for (auto& resultFuture : connectResultsFuture.get()) {
					auto connectResult = resultFuture.get();
					auto& connectionState = modifier.provisionConnectionState(state.ServiceId, connectResult.first);
					++connectionState.NumAttempts;
					if (net::PeerConnectCode::Accepted == connectResult.second) {
						++connectionState.NumSuccesses;
						connectionState.NumConsecutiveFailures = 0;
					} else {
						++connectionState.NumFailures;
						++connectionState.NumConsecutiveFailures;
					}
				}

				if (0 == state.PacketWriters.numActiveWriters())
					CATAPULT_LOG(warning) << "unable to connect to any nodes for service " << state.ServiceId;
			}

		private:
			AddState m_state;
		};
	}

	NodeSelector CreateNodeSelector(
			ionet::ServiceIdentifier serviceId,
			ionet::NodeRoles requiredRole,
			const config::NodeConfiguration::ConnectionsSubConfiguration& config,
			ionet::NodeContainer& nodes) {
		// 1. provision all existing nodes with a supported role
		nodes.modifier().addConnectionStates(serviceId, requiredRole);

		// 2. create a selector around the nodes and configuration
		extensions::NodeSelectionConfiguration selectionConfig{ serviceId, requiredRole, config.MaxConnections, config.MaxConnectionAge };
		return [&nodes, selectionConfig]() {
			return SelectNodes(nodes, selectionConfig);
		};
	}

	thread::Task CreateConnectPeersTask(
			ionet::NodeContainer& nodes,
			net::PacketWriters& packetWriters,
			ionet::ServiceIdentifier serviceId,
			ionet::NodeRoles requiredRole,
			const config::NodeConfiguration::ConnectionsSubConfiguration& config) {
		auto selector = CreateNodeSelector(serviceId, requiredRole, config, nodes);
		return CreateConnectPeersTask(nodes, packetWriters, serviceId, config, selector);
	}

	thread::Task CreateConnectPeersTask(
			ionet::NodeContainer& nodes,
			net::PacketWriters& packetWriters,
			ionet::ServiceIdentifier serviceId,
			const config::NodeConfiguration::ConnectionsSubConfiguration& config,
			const NodeSelector& selector) {
		auto ager = CreateNodeAger(serviceId, config, nodes);
		return thread::CreateNamedTask("connect peers task", [serviceId, ager, selector, &nodes, &packetWriters]() {
			// 1. age all connections
			ager(packetWriters.identities());

			// 2. select add and remove candidates
			auto result = selector();

			// 3. process remove candidates
			for (const auto& key : result.RemoveCandidates)
				packetWriters.closeOne(key);

			// 4. process add candidates
			AddCandidateProcessor processor({ nodes, packetWriters, serviceId });
			return processor.process(result.AddCandidates);
		});
	}

	// endregion

	// region CreateRemoveOnlyNodeSelector / CreateAgePeersTask

	RemoveOnlyNodeSelector CreateRemoveOnlyNodeSelector(
			ionet::ServiceIdentifier serviceId,
			const config::NodeConfiguration::ConnectionsSubConfiguration& config,
			ionet::NodeContainer& nodes) {
		// create a selector around the nodes and configuration
		extensions::NodeAgingConfiguration selectionConfig{ serviceId, config.MaxConnections, config.MaxConnectionAge };
		return [&nodes, selectionConfig]() {
			return SelectNodesForRemoval(nodes, selectionConfig);
		};
	}

	thread::Task CreateAgePeersTask(
			ionet::NodeContainer& nodes,
			net::ConnectionContainer& connectionContainer,
			ionet::ServiceIdentifier serviceId,
			const config::NodeConfiguration::ConnectionsSubConfiguration& config) {
		auto selector = CreateRemoveOnlyNodeSelector(serviceId, config, nodes);
		return CreateAgePeersTask(nodes, connectionContainer, serviceId, config, selector);
	}

	thread::Task CreateAgePeersTask(
			ionet::NodeContainer& nodes,
			net::ConnectionContainer& connectionContainer,
			ionet::ServiceIdentifier serviceId,
			const config::NodeConfiguration::ConnectionsSubConfiguration& config,
			const RemoveOnlyNodeSelector& selector) {
		auto ager = CreateNodeAger(serviceId, config, nodes);
		return thread::CreateNamedTask("age peers task", [serviceId, ager, selector, &connectionContainer]() {
			// 1. age all connections
			ager(connectionContainer.identities());

			// 2. select remove candidates
			auto removeCandidates = selector();

			// 3. process remove candidates
			for (const auto& key : removeCandidates)
				connectionContainer.closeOne(key);

			return thread::make_ready_future(thread::TaskResult::Continue);
		});
	}

	// endregion
}}
