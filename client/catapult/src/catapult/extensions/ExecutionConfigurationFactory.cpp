/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "ExecutionConfigurationFactory.h"
#include "catapult/model/NotificationPublisher.h"
#include "catapult/plugins/PluginManager.h"

namespace catapult { namespace extensions {

	chain::ExecutionConfiguration CreateExecutionConfiguration(const plugins::PluginManager& pluginManager) {
		chain::ExecutionConfiguration executionConfig;
		executionConfig.Network = pluginManager.config().Network;
		executionConfig.pObserver = pluginManager.createObserver();
		executionConfig.pValidator = pluginManager.createStatefulValidator();
		executionConfig.pNotificationPublisher = pluginManager.createNotificationPublisher();
		executionConfig.ResolverContextFactory = [&pluginManager](const auto& cache) {
			return pluginManager.createResolverContext(cache);
		};
		return executionConfig;
	}
}}
