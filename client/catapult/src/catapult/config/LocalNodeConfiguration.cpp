#include "LocalNodeConfiguration.h"
#include "ConfigurationFileLoader.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"
#include <boost/filesystem.hpp>
#include <iostream>

namespace catapult { namespace config {

	LocalNodeConfiguration::LocalNodeConfiguration(
			model::BlockChainConfiguration&& blockChainConfig,
			NodeConfiguration&& nodeConfig,
			LoggingConfiguration&& loggingConfig,
			UserConfiguration&& userConfig,
			std::vector<ionet::Node>&& peers)
			: BlockChain(std::move(blockChainConfig))
			, Node(std::move(nodeConfig))
			, Logging(std::move(loggingConfig))
			, User(std::move(userConfig))
			, Peers(std::move(peers))
	{}

	// region LocalNodeConfiguration file io

	namespace {
		auto LoadPeersConfiguration(const boost::filesystem::path& path, model::NetworkIdentifier networkIdentifier) {
			return LoadConfiguration(path, [networkIdentifier](const auto& filePath) {
				return LoadPeersFromPath(filePath, networkIdentifier);
			});
		}
	}

	LocalNodeConfiguration LocalNodeConfiguration::LoadFromPath(const boost::filesystem::path& resourcesPath) {
		auto blockChainConfig = LoadIniConfiguration<model::BlockChainConfiguration>(resourcesPath / "config-network.properties");
		auto peers = LoadPeersConfiguration(resourcesPath / "peers-mijin.json", blockChainConfig.Network.Identifier);
		return LocalNodeConfiguration(
				std::move(blockChainConfig),
				LoadIniConfiguration<NodeConfiguration>(resourcesPath / "config-node.properties"),
				LoadIniConfiguration<LoggingConfiguration>(resourcesPath / "config-log.properties"),
				LoadIniConfiguration<UserConfiguration>(resourcesPath / "config-user.properties"),
				std::move(peers));
	}

	// endregion
}}
