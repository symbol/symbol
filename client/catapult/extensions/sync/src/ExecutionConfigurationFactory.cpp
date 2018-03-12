#include "ExecutionConfigurationFactory.h"
#include "catapult/model/NotificationPublisher.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace sync {

	chain::ExecutionConfiguration CreateExecutionConfiguration(const plugins::PluginManager& pluginManager) {
		chain::ExecutionConfiguration executionConfig;
		executionConfig.Network = pluginManager.config().Network;
		executionConfig.pObserver = pluginManager.createObserver();
		executionConfig.pValidator = pluginManager.createStatefulValidator();
		executionConfig.pNotificationPublisher = pluginManager.createNotificationPublisher();
		return executionConfig;
	}
}}
