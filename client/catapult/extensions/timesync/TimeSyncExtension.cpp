/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

#include "src/TimeSynchronizationConfiguration.h"
#include "src/TimeSynchronizationService.h"
#include "src/TimeSynchronizationState.h"
#include "src/constants.h"
#include "catapult/extensions/ProcessBootstrapper.h"

namespace catapult { namespace timesync {

	namespace {
		void RegisterExtension(extensions::ProcessBootstrapper& bootstrapper) {
			auto& extensionManager = bootstrapper.extensionManager();

			// register network time provider
			auto pTimeSyncState = std::make_shared<TimeSynchronizationState>(
					bootstrapper.config().BlockChain.Network.EpochAdjustment,
					Clock_Adjustment_Threshold);
			extensionManager.setNetworkTimeSupplier([&timeSyncState = *pTimeSyncState]() {
				return timeSyncState.networkTime();
			});

			// register service(s)
			auto config = TimeSynchronizationConfiguration::LoadFromPath(bootstrapper.resourcesPath());
			extensionManager.addServiceRegistrar(CreateTimeSynchronizationServiceRegistrar(config, pTimeSyncState));
		}
	}
}}

extern "C" PLUGIN_API
void RegisterExtension(catapult::extensions::ProcessBootstrapper& bootstrapper) {
	catapult::timesync::RegisterExtension(bootstrapper);
}
