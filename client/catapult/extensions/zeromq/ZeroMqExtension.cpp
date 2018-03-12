#include "src/MessagingConfiguration.h"
#include "src/ZeroMqBlockChangeSubscriber.h"
#include "src/ZeroMqEntityPublisher.h"
#include "src/ZeroMqPtChangeSubscriber.h"
#include "src/ZeroMqTransactionStatusSubscriber.h"
#include "src/ZeroMqUtChangeSubscriber.h"
#include "catapult/extensions/LocalNodeBootstrapper.h"
#include "catapult/extensions/RootedService.h"
#include "catapult/model/NotificationPublisher.h"

namespace catapult { namespace zeromq {

	namespace {
		void RegisterExtension(extensions::LocalNodeBootstrapper& bootstrapper) {
			auto config = MessagingConfiguration::LoadFromPath(bootstrapper.resourcesPath());
			auto pZeroEntityPublisher = std::make_shared<ZeroMqEntityPublisher>(
					config.SubscriberPort,
					bootstrapper.pluginManager().createNotificationPublisher());

			// add a dummy service for extending service lifetimes
			bootstrapper.extensionManager().addServiceRegistrar(extensions::CreateRootedServiceRegistrar(
					pZeroEntityPublisher,
					"zeromq.publisher",
					extensions::ServiceRegistrarPhase::Initial));

			// register subscriptions
			auto& subscriptionManager = bootstrapper.subscriptionManager();
			subscriptionManager.addBlockChangeSubscriber(CreateZeroMqBlockChangeSubscriber(*pZeroEntityPublisher));
			subscriptionManager.addUtChangeSubscriber(CreateZeroMqUtChangeSubscriber(*pZeroEntityPublisher));
			subscriptionManager.addPtChangeSubscriber(CreateZeroMqPtChangeSubscriber(*pZeroEntityPublisher));
			subscriptionManager.addTransactionStatusSubscriber(CreateZeroMqTransactionStatusSubscriber(*pZeroEntityPublisher));
		}
	}
}}

extern "C" PLUGIN_API
void RegisterExtension(catapult::extensions::LocalNodeBootstrapper& bootstrapper) {
	catapult::zeromq::RegisterExtension(bootstrapper);
}
