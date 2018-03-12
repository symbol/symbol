#include "src/ApiNetworkPacketWritersService.h"
#include "catapult/extensions/LocalNodeBootstrapper.h"

namespace catapult { namespace eventsource {

	namespace {
		void RegisterExtension(extensions::LocalNodeBootstrapper& bootstrapper) {
			// register service(s)
			bootstrapper.extensionManager().addServiceRegistrar(CreateApiNetworkPacketWritersServiceRegistrar());
		}
	}
}}

extern "C" PLUGIN_API
void RegisterExtension(catapult::extensions::LocalNodeBootstrapper& bootstrapper) {
	catapult::eventsource::RegisterExtension(bootstrapper);
}
