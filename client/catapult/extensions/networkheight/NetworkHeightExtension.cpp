#include "src/NetworkHeightService.h"
#include "catapult/extensions/LocalNodeBootstrapper.h"

namespace catapult { namespace networkheight {

	namespace {
		void RegisterExtension(extensions::LocalNodeBootstrapper& bootstrapper) {
			// register service(s)
			bootstrapper.extensionManager().addServiceRegistrar(CreateNetworkHeightServiceRegistrar());
		}
	}
}}

extern "C" PLUGIN_API
void RegisterExtension(catapult::extensions::LocalNodeBootstrapper& bootstrapper) {
	catapult::networkheight::RegisterExtension(bootstrapper);
}
