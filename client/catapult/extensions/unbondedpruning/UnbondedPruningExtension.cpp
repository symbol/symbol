#include "src/UnbondedPruningService.h"
#include "catapult/extensions/LocalNodeBootstrapper.h"

namespace catapult { namespace unbondedpruning {

	namespace {
		void RegisterExtension(extensions::LocalNodeBootstrapper& bootstrapper) {
			// register service(s)
			bootstrapper.extensionManager().addServiceRegistrar(CreateUnbondedPruningServiceRegistrar());
		}
	}
}}

extern "C" PLUGIN_API
void RegisterExtension(catapult::extensions::LocalNodeBootstrapper& bootstrapper) {
	catapult::unbondedpruning::RegisterExtension(bootstrapper);
}
