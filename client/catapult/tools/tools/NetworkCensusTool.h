#pragma once
#include "Tool.h"
#include "ToolConfigurationUtils.h"
#include "ToolNetworkUtils.h"
#include "catapult/ionet/Node.h"
#include "catapult/thread/FutureUtils.h"

namespace catapult { namespace tools {

	/// A base class for a tool that performs a network census by communicating with all nodes.
	template<typename TNodeInfo>
	class NetworkCensusTool : public Tool {
	public:
		/// A node info shared pointer.
		using NodeInfoPointer = std::shared_ptr<TNodeInfo>;

		/// A node info shared pointer future.
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
			auto p2pNodes = LoadPeers(m_resourcesPath, config.BlockChain.Network.Identifier);
			auto apiNodes = LoadOptionalApiPeers(m_resourcesPath, config.BlockChain.Network.Identifier);

			MultiNodeConnector connector;
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

				this->processNodeInfos(nodeInfos);
			});

			finalFuture.get();
			return 0;
		}

	private:
		NodeInfoFuture createNodeInfoFuture(MultiNodeConnector& connector, const ionet::Node& node) {
			auto pNodeInfo = std::make_shared<TNodeInfo>(node);
			return thread::compose(connector.connect(node), [this, node, pNodeInfo, &connector](auto&& ioFuture) {
				try {
					auto pIo = ioFuture.get();
					auto infoFutures = this->getNodeInfoFutures(connector.pool(), *pIo, *pNodeInfo);

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
		/// Gets all futures to fill \a nodeInfo using \a pool and \a io.
		virtual std::vector<thread::future<bool>> getNodeInfoFutures(
				thread::IoServiceThreadPool& pool,
				ionet::PacketIo& io,
				TNodeInfo& nodeInfo) = 0;

		/// Processes \a nodeInfos after all futures complete .
		virtual void processNodeInfos(const std::vector<NodeInfoPointer>& nodeInfos) = 0;

	private:
		std::string m_censusName;
		std::string m_resourcesPath;
	};
}}
