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

#include "finalization/src/io/FileProofStorage.h"
#include "src/FinalizationBootstrapperService.h"
#include "src/FinalizationConfiguration.h"
#include "src/FinalizationMessageProcessingService.h"
#include "src/FinalizationOrchestratorService.h"
#include "src/FinalizationService.h"
#include "src/FinalizationSyncSourceService.h"
#include "catapult/config/ConfigurationFileLoader.h"
#include "catapult/extensions/ProcessBootstrapper.h"

namespace catapult { namespace finalization {

	namespace {
		void RegisterExtension(extensions::ProcessBootstrapper& bootstrapper) {
			const auto& resourcesPath = bootstrapper.resourcesPath();
			auto config = FinalizationConfiguration::LoadFromPath(resourcesPath);
			config.VotingSetGrouping = bootstrapper.config().BlockChain.VotingSetGrouping;

			auto pProofStorage = std::make_unique<io::FileProofStorage>(bootstrapper.config().User.DataDirectory);

			// register other services
			auto& extensionManager = bootstrapper.extensionManager();
			extensionManager.addServiceRegistrar(CreateFinalizationBootstrapperServiceRegistrar(config, std::move(pProofStorage)));
			extensionManager.addServiceRegistrar(CreateFinalizationBootstrapperPhaseTwoServiceRegistrar());
			extensionManager.addServiceRegistrar(CreateFinalizationServiceRegistrar(config));
			extensionManager.addServiceRegistrar(CreateFinalizationSyncSourceServiceRegistrar(config.EnableVoting));

			if (config.EnableVoting) {
				extensionManager.addServiceRegistrar(CreateFinalizationMessageProcessingServiceRegistrar(config));
				extensionManager.addServiceRegistrar(CreateFinalizationOrchestratorServiceRegistrar(config));
			}
		}
	}
}}

extern "C" PLUGIN_API
void RegisterExtension(catapult::extensions::ProcessBootstrapper& bootstrapper) {
	catapult::finalization::RegisterExtension(bootstrapper);
}
