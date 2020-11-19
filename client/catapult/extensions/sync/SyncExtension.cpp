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

#include "src/DispatcherService.h"
#include "src/NetworkPacketWritersService.h"
#include "src/SchedulerService.h"
#include "src/SyncService.h"
#include "src/TasksConfiguration.h"
#include "catapult/extensions/ProcessBootstrapper.h"

namespace catapult { namespace sync {

	namespace {
		void RegisterExtension(extensions::ProcessBootstrapper& bootstrapper) {
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
void RegisterExtension(catapult::extensions::ProcessBootstrapper& bootstrapper) {
	catapult::sync::RegisterExtension(bootstrapper);
}
