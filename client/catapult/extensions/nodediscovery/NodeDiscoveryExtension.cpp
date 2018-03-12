#include "src/NodeDiscoveryService.h"
#include "catapult/extensions/LocalNodeBootstrapper.h"
#include "catapult/ionet/NetworkNode.h"

namespace catapult { namespace nodediscovery {

	namespace {
		void RegisterExtension(extensions::LocalNodeBootstrapper& bootstrapper) {
			auto pLocalNetworkNode = ionet::PackNode(config::ToLocalNode(bootstrapper.config()));

			// register service(s)
			bootstrapper.extensionManager().addServiceRegistrar(CreateNodeDiscoveryServiceRegistrar(std::move(pLocalNetworkNode)));
		}
	}
}}

extern "C" PLUGIN_API
void RegisterExtension(catapult::extensions::LocalNodeBootstrapper& bootstrapper) {
	catapult::nodediscovery::RegisterExtension(bootstrapper);
}
