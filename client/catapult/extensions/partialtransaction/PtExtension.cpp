#include "src/PtBootstrapperService.h"
#include "src/PtConfiguration.h"
#include "src/PtDispatcherService.h"
#include "src/PtService.h"
#include "src/PtSyncSourceService.h"
#include "catapult/config/ConfigurationFileLoader.h"
#include "catapult/extensions/LocalNodeBootstrapper.h"
#include <boost/filesystem/path.hpp>

namespace catapult { namespace partialtransaction {

	namespace {
		void RegisterExtension(extensions::LocalNodeBootstrapper& bootstrapper) {
			const auto& resourcesPath = bootstrapper.resourcesPath();
			auto config = PtConfiguration::LoadFromPath(resourcesPath);
			auto ptCacheOptions = cache::MemoryCacheOptions(config.CacheMaxResponseSize.bytes(), config.CacheMaxSize);
			AddStaticNodesFromPath(bootstrapper, (boost::filesystem::path(resourcesPath) / "peers-api.json").generic_string());

			// create and register the pt cache (it is optional, so not in server state)
			auto& extensionManager = bootstrapper.extensionManager();
			auto& subscriptionManager = bootstrapper.subscriptionManager();
			extensionManager.addServiceRegistrar(CreatePtBootstrapperServiceRegistrar([&subscriptionManager, ptCacheOptions]() {
				return subscriptionManager.createPtCache(ptCacheOptions);
			}));

			// register other services
			extensionManager.addServiceRegistrar(CreatePtDispatcherServiceRegistrar());
			extensionManager.addServiceRegistrar(CreatePtSyncSourceServiceRegistrar());
			extensionManager.addServiceRegistrar(CreatePtServiceRegistrar());
		}
	}
}}

extern "C" PLUGIN_API
void RegisterExtension(catapult::extensions::LocalNodeBootstrapper& bootstrapper) {
	catapult::partialtransaction::RegisterExtension(bootstrapper);
}
