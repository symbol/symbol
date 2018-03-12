#include "src/AddressExtractionUtSubscriber.h"
#include "catapult/extensions/LocalNodeBootstrapper.h"

namespace catapult { namespace addressextraction {

	namespace {
		void RegisterExtension(extensions::LocalNodeBootstrapper& bootstrapper) {
			auto pNotificationPublisher = bootstrapper.pluginManager().createNotificationPublisher();

			// register subscriber
			auto& subscriptionManager = bootstrapper.subscriptionManager();
			subscriptionManager.addUtChangeSubscriber(CreateAddressExtractionChangeSubscriber(std::move(pNotificationPublisher)));
		}
	}
}}

extern "C" PLUGIN_API
void RegisterExtension(catapult::extensions::LocalNodeBootstrapper& bootstrapper) {
	catapult::addressextraction::RegisterExtension(bootstrapper);
}
