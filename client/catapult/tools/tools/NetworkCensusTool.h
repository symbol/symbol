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

#pragma once
#include "Tool.h"
#include "ToolConfigurationUtils.h"
#include "ToolKeys.h"
#include "ToolNetworkUtils.h"
#include "catapult/ionet/Node.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/thread/FutureUtils.h"

namespace catapult { namespace tools {

	/// Base class for a tool that performs a network census by communicating with all nodes.
	template<typename TNodeInfo>
	class NetworkCensusTool : public Tool {
	public:
		/// Node info shared pointer.
		using NodeInfoPointer = std::shared_ptr<TNodeInfo>;

		/// Node info shared pointer future.
		using NodeInfoFuture = thread::future<NodeInfoPointer>;

	public:
		/// Creates a census tool with census name (\a censusName).
		explicit NetworkCensusTool(const std::string& censusName) : m_censusName(censusName)
		{}

	public:
		std::string name() const override final {
			return "Catapult Block Chain " + m_censusName + " Tool";
		}

		void prepareOptions(OptionsBuilder& optionsBuilder, OptionsPositional& positional) override final {
			optionsBuilder("resources,r",
					OptionsValue<std::string>(m_resourcesPath)->default_value(".."),
					"the path to the resources directory");
			positional.add("resources", -1);
		}

		int run(const Options&) override final {
			auto config = LoadConfiguration(m_resourcesPath);
			auto networkFingerprint = model::UniqueNetworkFingerprint(
					config.BlockChain.Network.Identifier,
					config.BlockChain.Network.GenerationHashSeed);
			auto p2pNodes = LoadPeers(m_resourcesPath, networkFingerprint);
			auto apiNodes = LoadOptionalApiPeers(m_resourcesPath, networkFingerprint);

			MultiNodeConnector connector(config.User.CertificateDirectory);
			std::vector<NodeInfoFuture> nodeInfoFutures;
			auto addNodeInfoFutures = [this, &connector, &nodeInfoFutures](const auto& nodes) {
				for (const auto& node : nodes) {
					CATAPULT_LOG(debug) << "preparing to get stats from node " << node;
					nodeInfoFutures.push_back(this->createNodeInfoFuture(connector, node));
				}
			};

			addNodeInfoFutures(p2pNodes);
			addNodeInfoFutures(apiNodes);

			auto finalFuture = thread::when_all(std::move(nodeInfoFutures)).then([this](auto&& allFutures) {
				std::vector<NodeInfoPointer> nodeInfos;
				for (auto& nodeInfoFuture : allFutures.get())
					nodeInfos.push_back(nodeInfoFuture.get());

				return this->processNodeInfos(nodeInfos);
			});

			auto result = utils::checked_cast<size_t, unsigned int>(finalFuture.get());
			return std::min(static_cast<int>(result), 255);
		}

	private:
		NodeInfoFuture createNodeInfoFuture(MultiNodeConnector& connector, const ionet::Node& node) {
			auto pNodeInfo = std::make_shared<TNodeInfo>(node);
			return thread::compose(connector.connect(node), [this, node, pNodeInfo, &connector](auto&& socketInfoFuture) {
				try {
					auto socketInfo = socketInfoFuture.get();
					auto pIo = socketInfo.socket()->buffered();
					auto nodeIdentity = model::NodeIdentity{ socketInfo.publicKey(), socketInfo.host() };
					auto infoFutures = this->getNodeInfoFutures(connector.pool(), *pIo, nodeIdentity, *pNodeInfo);

					// capture pIo so that it stays alive until all dependent futures are complete
					return thread::when_all(std::move(infoFutures)).then([pIo, pNodeInfo](auto&&) {
						return pNodeInfo;
					});
				} catch (...) {
					// suppress
					CATAPULT_LOG(error) << node << " appears to be offline";
					return thread::make_ready_future(NodeInfoPointer(pNodeInfo));
				}
			});
		}

	private:
		/// Gets all futures to fill \a nodeInfo for \a nodeIdentity using \a pool and \a io.
		virtual std::vector<thread::future<bool>> getNodeInfoFutures(
				thread::IoThreadPool& pool,
				ionet::PacketIo& io,
				const model::NodeIdentity& nodeIdentity,
				TNodeInfo& nodeInfo) = 0;

		/// Processes \a nodeInfos after all futures complete.
		virtual size_t processNodeInfos(const std::vector<NodeInfoPointer>& nodeInfos) = 0;

	private:
		std::string m_censusName;
		std::string m_resourcesPath;
	};
}}
