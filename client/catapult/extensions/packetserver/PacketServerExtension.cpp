#include "src/NetworkPacketReadersService.h"
#include "catapult/extensions/LocalNodeBootstrapper.h"

namespace catapult { namespace packetserver {

	namespace {
		void RegisterExtension(extensions::LocalNodeBootstrapper& bootstrapper) {
			// register service(s)
			bootstrapper.extensionManager().addServiceRegistrar(CreateNetworkPacketReadersServiceRegistrar());
		}
	}
}}

extern "C" PLUGIN_API
void RegisterExtension(catapult::extensions::LocalNodeBootstrapper& bootstrapper) {
	catapult::packetserver::RegisterExtension(bootstrapper);
}
