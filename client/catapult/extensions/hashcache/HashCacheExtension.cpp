#include "src/HashCacheService.h"
#include "catapult/extensions/LocalNodeBootstrapper.h"

namespace catapult { namespace hashcache {

	namespace {
		void RegisterExtension(extensions::LocalNodeBootstrapper& bootstrapper) {
			auto& extensionManager = bootstrapper.extensionManager();

			// register hash cache
			extensionManager.registerSystemPlugin("catapult.plugins.hashcache");
			extensionManager.addServiceRegistrar(CreateHashCacheServiceRegistrar());
		}
	}
}}

extern "C" PLUGIN_API
void RegisterExtension(catapult::extensions::LocalNodeBootstrapper& bootstrapper) {
	catapult::hashcache::RegisterExtension(bootstrapper);
}
