#include "src/FileBlockChainStorage.h"
#include "catapult/extensions/LocalNodeBootstrapper.h"

namespace catapult { namespace filechain {

	namespace {
		void RegisterExtension(extensions::LocalNodeBootstrapper& bootstrapper) {
			// register storage
			bootstrapper.extensionManager().setBlockChainStorage(CreateFileBlockChainStorage());
		}
	}
}}

extern "C" PLUGIN_API
void RegisterExtension(catapult::extensions::LocalNodeBootstrapper& bootstrapper) {
	catapult::filechain::RegisterExtension(bootstrapper);
}
