#include "src/DispatcherService.h"
#include "src/NetworkPacketWritersService.h"
#include "src/SchedulerService.h"
#include "src/SyncService.h"
#include "src/TasksConfiguration.h"
#include "catapult/extensions/LocalNodeBootstrapper.h"

namespace catapult { namespace sync {

	namespace {
		void RegisterExtension(extensions::LocalNodeBootstrapper& bootstrapper) {
			const auto& resourcesPath = bootstrapper.resourcesPath();
			auto tasksConfig = TasksConfiguration::LoadFromPath(resourcesPath);

			// register service(s)
			auto& extensionManager = bootstrapper.extensionManager();
			extensionManager.addServiceRegistrar(CreateDispatcherServiceRegistrar());
			extensionManager.addServiceRegistrar(CreateNetworkPacketWritersServiceRegistrar());
			extensionManager.addServiceRegistrar(CreateSchedulerServiceRegistrar(tasksConfig));
			extensionManager.addServiceRegistrar(CreateSyncServiceRegistrar());
		}
	}
}}

extern "C" PLUGIN_API
void RegisterExtension(catapult::extensions::LocalNodeBootstrapper& bootstrapper) {
	catapult::sync::RegisterExtension(bootstrapper);
}
