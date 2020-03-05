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

#include "CatapultConfiguration.h"
#include "CatapultKeys.h"
#include "ConfigurationFileLoader.h"
#include "catapult/crypto/OpensslKeyUtils.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace config {

	// region CatapultConfiguration

	namespace {
		std::string Qualify(const std::string& name) {
			std::ostringstream out;
			out << "config-" << name << ".properties";
			return out.str();
		}

		std::string HostQualify(const std::string& name, const std::string& host) {
			std::ostringstream out;
			out << "config-" << name << "-" << host << ".properties";
			return out.str();
		}
	}

	CatapultConfiguration::CatapultConfiguration(
			model::BlockChainConfiguration&& blockChainConfig,
			NodeConfiguration&& nodeConfig,
			LoggingConfiguration&& loggingConfig,
			UserConfiguration&& userConfig,
			ExtensionsConfiguration&& extensionsConfig,
			InflationConfiguration&& inflationConfig)
			: BlockChain(std::move(blockChainConfig))
			, Node(std::move(nodeConfig))
			, Logging(std::move(loggingConfig))
			, User(std::move(userConfig))
			, Extensions(std::move(extensionsConfig))
			, Inflation(std::move(inflationConfig))
	{}

	CatapultConfiguration CatapultConfiguration::LoadFromPath(
			const boost::filesystem::path& resourcesPath,
			const std::string& extensionsHost) {
		return CatapultConfiguration(
				LoadIniConfiguration<model::BlockChainConfiguration>(resourcesPath / Qualify("network")),
				LoadIniConfiguration<NodeConfiguration>(resourcesPath / Qualify("node")),
				LoadIniConfiguration<LoggingConfiguration>(resourcesPath / HostQualify("logging", extensionsHost)),
				LoadIniConfiguration<UserConfiguration>(resourcesPath / Qualify("user")),
				LoadIniConfiguration<ExtensionsConfiguration>(resourcesPath / HostQualify("extensions", extensionsHost)),
				LoadIniConfiguration<InflationConfiguration>(resourcesPath / Qualify("inflation")));
	}

	// endregion

	ionet::Node ToLocalNode(const CatapultConfiguration& config) {
		const auto& localNodeConfig = config.Node.Local;

		auto identityKey = crypto::ReadPublicKeyFromPublicKeyPemFile(GetCaPublicKeyPemFilename(config.User.CertificateDirectory));

		auto endpoint = ionet::NodeEndpoint();
		endpoint.Host = localNodeConfig.Host;
		endpoint.Port = config.Node.Port;

		auto networkFingerprint = model::UniqueNetworkFingerprint(
				config.BlockChain.Network.Identifier,
				config.BlockChain.Network.GenerationHash);
		auto metadata = ionet::NodeMetadata(networkFingerprint);
		metadata.Name = localNodeConfig.FriendlyName;
		metadata.Version = ionet::NodeVersion(localNodeConfig.Version);
		metadata.Roles = localNodeConfig.Roles;

		return ionet::Node({ identityKey, "127.0.0.1" }, endpoint, metadata);
	}
}}
