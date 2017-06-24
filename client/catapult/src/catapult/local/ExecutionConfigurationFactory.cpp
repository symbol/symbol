#include "ExecutionConfigurationFactory.h"
#include "catapult/model/NotificationPublisher.h"
#include "catapult/plugins/PluginManager.h"
#include "catapult/validators/SequentialValidationPolicy.h"

namespace catapult { namespace local {

	chain::ExecutionConfiguration CreateExecutionConfiguration(const plugins::PluginManager& pluginManager) {
		chain::ExecutionConfiguration executionConfig;
		executionConfig.Network = pluginManager.config().Network;
		executionConfig.pObserver = pluginManager.createObserver();
		executionConfig.pValidator = pluginManager.createStatefulValidator();
		executionConfig.pNotificationPublisher = CreateNotificationPublisher(pluginManager.transactionRegistry());
		return executionConfig;
	}
}}
