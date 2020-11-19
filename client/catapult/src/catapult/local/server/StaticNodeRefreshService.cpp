/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "StaticNodeRefreshService.h"
#include "catapult/config/CatapultKeys.h"
#include "catapult/extensions/NetworkUtils.h"
#include "catapult/extensions/ServiceLocator.h"
#include "catapult/extensions/ServiceState.h"
#include "catapult/ionet/NodeContainer.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/net/ServerConnector.h"

namespace catapult { namespace local {

	namespace {
		thread::Task CreateRefreshTask(
				const std::vector<ionet::Node>& staticNodes,
				const Key& bootPublicKey,
				net::ServerConnector& connector,
				ionet::NodeContainer& nodes) {
			return thread::CreateNamedTask("static node refresh task", [&staticNodes, &bootPublicKey, &connector, &nodes]() {
				std::vector<thread::future<bool>> futures;
				for (const auto& node : staticNodes) {
					// skip requests to self
					if (bootPublicKey == node.identity().PublicKey)
						continue;

					auto pPromise = std::make_shared<thread::promise<bool>>();
					futures.push_back(pPromise->get_future());

					connector.connect(node, [pPromise, &node, &nodes](auto code, const auto& socketInfo) {
						if (net::PeerConnectCode::Accepted != code) {
							CATAPULT_LOG(warning) << "unable to resolve and connect to static node " << node;
							pPromise->set_value(false);
							return;
						}

						auto resolvedNodeIdentity = model::NodeIdentity{ node.identity().PublicKey, socketInfo.host() };
						auto resolvedNode = ionet::Node(resolvedNodeIdentity, node.endpoint(), node.metadata());
						nodes.modifier().add(resolvedNode, ionet::NodeSource::Static);
						pPromise->set_value(true);
					});
				}

				return futures.empty()
						? thread::make_ready_future(thread::TaskResult::Continue)
						: thread::when_all(std::move(futures)).then([](auto&&) { return thread::TaskResult::Continue; });
			});
		}

		class StaticNodeRefreshServiceRegistrar : public extensions::ServiceRegistrar {
		public:
			explicit StaticNodeRefreshServiceRegistrar(const std::vector<ionet::Node>& staticNodes) : m_staticNodes(staticNodes)
			{}

		public:
			extensions::ServiceRegistrarInfo info() const override {
				return { "StaticNodeRefresh", extensions::ServiceRegistrarPhase::Initial };
			}

			void registerServiceCounters(extensions::ServiceLocator&) override {
				// no additional counters
			}

			void registerServices(extensions::ServiceLocator& locator, extensions::ServiceState& state) override {
				// register services
				auto connectionSettings = extensions::GetConnectionSettings(state.config());
				auto pServiceGroup = state.pool().pushServiceGroup("static_node_refresh");

				auto pServerConnector = pServiceGroup->pushService(
						net::CreateServerConnector,
						locator.keys().caPublicKey(),
						connectionSettings,
						"static node refresh");
				locator.registerService("snr.server_connector", pServerConnector);

				// add task
				state.tasks().push_back(CreateRefreshTask(m_staticNodes, locator.keys().caPublicKey(), *pServerConnector, state.nodes()));
			}

		private:
			const std::vector<ionet::Node>& m_staticNodes;
		};
	}

	DECLARE_SERVICE_REGISTRAR(StaticNodeRefresh)(const std::vector<ionet::Node>& staticNodes) {
		return std::make_unique<StaticNodeRefreshServiceRegistrar>(staticNodes);
	}
}}
