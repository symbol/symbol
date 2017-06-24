#include "ToolNetworkUtils.h"
#include "NetworkConnections.h"
#include "Waits.h"
#include "catapult/api/RemoteChainApi.h"
#include "catapult/config/LocalNodeConfiguration.h"

namespace catapult { namespace tools {

	Height GetHeight(const NetworkConnections& connections) {
		auto ioPair = connections.pickOne();
		if (!ioPair)
			CATAPULT_THROW_RUNTIME_ERROR("no connection to network available");

		auto pChainApi = api::CreateRemoteChainApi(ioPair.io());
		return pChainApi->chainInfo().get().Height;
	}

	bool WaitForBlocks(const NetworkConnections& connections, size_t numBlocks) {
		try {
			CATAPULT_LOG(info) << "waiting for " << numBlocks << " blocks...";
			auto currentHeight = GetHeight(connections);
			auto endHeight = currentHeight + Height(numBlocks);
			CATAPULT_LOG(info) << "waiting for block " << endHeight << ", current block is " << currentHeight;
			while (currentHeight < endHeight) {
				Sleep();
				Height oldHeight = currentHeight;
				currentHeight = GetHeight(connections);
				if (currentHeight != oldHeight)
					CATAPULT_LOG(info) << "block " << currentHeight << " was harvested";
			}

			return true;
		} catch (const std::exception& e) {
			CATAPULT_LOG(error) << "exception thrown while waiting for new blocks: " << e.what();
			throw;
		}
	}

	config::LocalNodeConfiguration LoadConfiguration(const std::string& resourcesPathStr) {
		boost::filesystem::path resourcesPath = resourcesPathStr;
		resourcesPath /= "resources";
		std::cout << "loading resources from " << resourcesPath << std::endl;
		return config::LocalNodeConfiguration::LoadFromPath(resourcesPath);
	}
}}
