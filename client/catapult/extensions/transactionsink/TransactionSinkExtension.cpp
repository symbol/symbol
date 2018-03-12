#include "src/TransactionSinkService.h"
#include "catapult/extensions/LocalNodeBootstrapper.h"

namespace catapult { namespace transactionsink {

	namespace {
		void RegisterExtension(extensions::LocalNodeBootstrapper& bootstrapper) {
			// register service(s)
			bootstrapper.extensionManager().addServiceRegistrar(CreateTransactionSinkServiceRegistrar());
		}
	}
}}

extern "C" PLUGIN_API
void RegisterExtension(catapult::extensions::LocalNodeBootstrapper& bootstrapper) {
	catapult::transactionsink::RegisterExtension(bootstrapper);
}
