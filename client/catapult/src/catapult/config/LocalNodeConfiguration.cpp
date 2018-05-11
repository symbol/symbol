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

#include "LocalNodeConfiguration.h"
#include "ConfigurationFileLoader.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace config {

	// region LocalNodeConfiguration

	LocalNodeConfiguration::LocalNodeConfiguration(
			model::BlockChainConfiguration&& blockChainConfig,
			NodeConfiguration&& nodeConfig,
			LoggingConfiguration&& loggingConfig,
			UserConfiguration&& userConfig)
			: BlockChain(std::move(blockChainConfig))
			, Node(std::move(nodeConfig))
			, Logging(std::move(loggingConfig))
			, User(std::move(userConfig))
	{}

	LocalNodeConfiguration LocalNodeConfiguration::LoadFromPath(const boost::filesystem::path& resourcesPath) {
		auto blockChainConfig = LoadIniConfiguration<model::BlockChainConfiguration>(resourcesPath / "config-network.properties");
		return LocalNodeConfiguration(
				std::move(blockChainConfig),
				LoadIniConfiguration<NodeConfiguration>(resourcesPath / "config-node.properties"),
				LoadIniConfiguration<LoggingConfiguration>(resourcesPath / "config-logging.properties"),
				LoadIniConfiguration<UserConfiguration>(resourcesPath / "config-user.properties"));
	}

	// endregion

	ionet::Node ToLocalNode(const LocalNodeConfiguration& config) {
		const auto& localNodeConfig = config.Node.Local;

		auto identityKey = crypto::KeyPair::FromString(config.User.BootKey).publicKey();

		auto endpoint = ionet::NodeEndpoint();
		endpoint.Host = localNodeConfig.Host;
		endpoint.Port = config.Node.Port;

		auto metadata = ionet::NodeMetadata(config.BlockChain.Network.Identifier);
		metadata.Name = localNodeConfig.FriendlyName;
		metadata.Version = ionet::NodeVersion(localNodeConfig.Version);
		metadata.Roles = localNodeConfig.Roles;

		return ionet::Node(identityKey, endpoint, metadata);
	}
}}
