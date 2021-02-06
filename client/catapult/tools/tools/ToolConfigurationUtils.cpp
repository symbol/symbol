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

#include "ToolConfigurationUtils.h"
#include "catapult/config/ConfigurationFileLoader.h"

namespace catapult { namespace tools {

	void AddResourcesOption(OptionsBuilder& optionsBuilder) {
		optionsBuilder("resources,r",
				OptionsValue<std::string>()->default_value(".."),
				"path to the resources directory");
	}

	std::string GetResourcesOptionValue(const Options& options) {
		return options["resources"].as<std::string>();
	}

	config::CatapultConfiguration LoadConfiguration(const std::string& resourcesPath) {
		auto configPath = std::filesystem::path(resourcesPath) / "resources";
		std::cout << "loading resources from " << configPath << std::endl;
		return config::CatapultConfiguration::LoadFromPath(configPath, "server");
	}

	std::vector<ionet::Node> LoadOptionalApiPeers(
			const std::string& resourcesPath,
			const model::UniqueNetworkFingerprint& networkFingerprint) {
		std::vector<ionet::Node> apiNodes;
		auto apiPeersFilename = std::filesystem::path(resourcesPath) / "resources" / "peers-api.json";
		if (std::filesystem::exists(apiPeersFilename))
			apiNodes = config::LoadPeersConfiguration(apiPeersFilename, networkFingerprint);

		return apiNodes;
	}

	std::vector<ionet::Node> LoadPeers(const std::string& resourcesPath, const model::UniqueNetworkFingerprint& networkFingerprint) {
		auto peersFilename = std::filesystem::path(resourcesPath) / "resources" / "peers-p2p.json";
		return config::LoadPeersConfiguration(peersFilename, networkFingerprint);
	}
}}
