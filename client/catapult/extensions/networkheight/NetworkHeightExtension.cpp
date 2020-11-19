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

#include "src/NetworkHeightConfiguration.h"
#include "src/NetworkHeightService.h"
#include "catapult/extensions/ProcessBootstrapper.h"

namespace catapult { namespace networkheight {

	namespace {
		void RegisterExtension(extensions::ProcessBootstrapper& bootstrapper) {
			// register service(s)
			auto config = NetworkHeightConfiguration::LoadFromPath(bootstrapper.resourcesPath());
			bootstrapper.extensionManager().addServiceRegistrar(CreateNetworkHeightServiceRegistrar(config));
		}
	}
}}

extern "C" PLUGIN_API
void RegisterExtension(catapult::extensions::ProcessBootstrapper& bootstrapper) {
	catapult::networkheight::RegisterExtension(bootstrapper);
}
