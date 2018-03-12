#include "src/DiagnosticsService.h"
#include "catapult/extensions/LocalNodeBootstrapper.h"

namespace catapult { namespace diagnostics {

	namespace {
		void RegisterExtension(extensions::LocalNodeBootstrapper& bootstrapper) {
			// register service(s)
			bootstrapper.extensionManager().addServiceRegistrar(CreateDiagnosticsServiceRegistrar());
		}
	}
}}

extern "C" PLUGIN_API
void RegisterExtension(catapult::extensions::LocalNodeBootstrapper& bootstrapper) {
	catapult::diagnostics::RegisterExtension(bootstrapper);
}
