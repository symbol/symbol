#include "src/SyncSourceService.h"
#include "catapult/extensions/LocalNodeBootstrapper.h"

namespace catapult { namespace syncsource {

	namespace {
		void RegisterExtension(extensions::LocalNodeBootstrapper& bootstrapper) {
			// register service(s)
			bootstrapper.extensionManager().addServiceRegistrar(CreateSyncSourceServiceRegistrar());
		}
	}
}}

extern "C" PLUGIN_API
void RegisterExtension(catapult::extensions::LocalNodeBootstrapper& bootstrapper) {
	catapult::syncsource::RegisterExtension(bootstrapper);
}
