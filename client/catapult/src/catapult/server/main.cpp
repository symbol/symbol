#include "ServerMain.h"
#include "catapult/extensions/LocalNodeBootstrapper.h"
#include "catapult/local/BasicLocalNode.h"

int main(int argc, const char** argv) {
	using namespace catapult;
	return server::ServerMain(argc, argv, [argc, argv](auto&& config, const auto& keyPair) {
		// create bootstrapper
		auto resourcesPath = server::GetResourcesPath(argc, argv).generic_string();
		auto pBootstrapper = std::make_unique<extensions::LocalNodeBootstrapper>(config, resourcesPath, "server");
		AddStaticNodesFromPath(*pBootstrapper, (boost::filesystem::path(resourcesPath) / "peers-p2p.json").generic_string());

		// register extension(s)
		pBootstrapper->loadExtensions();

		// create the local node
		return local::CreateBasicLocalNode(keyPair, std::move(pBootstrapper));
	});
}
