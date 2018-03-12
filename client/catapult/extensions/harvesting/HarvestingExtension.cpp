#include "src/HarvestingConfiguration.h"
#include "src/HarvestingService.h"
#include "src/ValidateHarvestingConfiguration.h"
#include "catapult/extensions/LocalNodeBootstrapper.h"

namespace catapult { namespace harvesting {

	namespace {
		void RegisterExtension(extensions::LocalNodeBootstrapper& bootstrapper) {
			auto config = HarvestingConfiguration::LoadFromPath(bootstrapper.resourcesPath());
			ValidateHarvestingConfiguration(config);

			bootstrapper.extensionManager().addServiceRegistrar(CreateHarvestingServiceRegistrar(config));
		}
	}
}}

extern "C" PLUGIN_API
void RegisterExtension(catapult::extensions::LocalNodeBootstrapper& bootstrapper) {
	catapult::harvesting::RegisterExtension(bootstrapper);
}
