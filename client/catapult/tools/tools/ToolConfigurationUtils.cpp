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

#include "ToolConfigurationUtils.h"
#include "catapult/config/ConfigurationFileLoader.h"

namespace catapult { namespace tools {

	config::CatapultConfiguration LoadConfiguration(const std::string& resourcesPathStr) {
		boost::filesystem::path resourcesPath = resourcesPathStr;
		resourcesPath /= "resources";
		std::cout << "loading resources from " << resourcesPath << std::endl;
		return config::CatapultConfiguration::LoadFromPath(resourcesPath, "server");
	}

	std::vector<ionet::Node> LoadOptionalApiPeers(
			const std::string& resourcesPath,
			const model::UniqueNetworkFingerprint& networkFingerprint) {
		std::vector<ionet::Node> apiNodes;
		auto apiPeersFilename = boost::filesystem::path(resourcesPath) / "resources" / "peers-api.json";
		if (boost::filesystem::exists(apiPeersFilename))
			apiNodes = config::LoadPeersConfiguration(apiPeersFilename, networkFingerprint);

		return apiNodes;
	}

	std::vector<ionet::Node> LoadPeers(const std::string& resourcesPath, const model::UniqueNetworkFingerprint& networkFingerprint) {
		auto peersFilename = boost::filesystem::path(resourcesPath) / "resources" / "peers-p2p.json";
		return config::LoadPeersConfiguration(peersFilename, networkFingerprint);
	}
}}
