#include "ToolConfigurationUtils.h"
#include "catapult/config/ConfigurationFileLoader.h"

namespace catapult { namespace tools {

	config::LocalNodeConfiguration LoadConfiguration(const std::string& resourcesPathStr) {
		boost::filesystem::path resourcesPath = resourcesPathStr;
		resourcesPath /= "resources";
		std::cout << "loading resources from " << resourcesPath << std::endl;
		return config::LocalNodeConfiguration::LoadFromPath(resourcesPath);
	}

	std::vector<ionet::Node> LoadOptionalApiPeers(const std::string& resourcesPath, model::NetworkIdentifier networkIdentifier) {
		std::vector<ionet::Node> apiNodes;
		auto apiPeersFilename = boost::filesystem::path(resourcesPath) / "resources" / "peers-api.json";
		if (boost::filesystem::exists(apiPeersFilename))
			apiNodes = config::LoadPeersConfiguration(apiPeersFilename, networkIdentifier);

		return apiNodes;
	}

	std::vector<ionet::Node> LoadPeers(const std::string& resourcesPath, model::NetworkIdentifier networkIdentifier) {
		auto peersFilename = boost::filesystem::path(resourcesPath) / "resources" / "peers-p2p.json";
		return config::LoadPeersConfiguration(peersFilename, networkIdentifier);
	}
}}
