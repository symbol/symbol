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

#pragma once
#include "catapult/plugins/PluginModule.h"
#include "catapult/utils/ExceptionLogging.h"
#include "catapult/exceptions.h"
#include <memory>
#include <vector>

namespace catapult { namespace extensions { class ProcessBootstrapper; } }

namespace catapult { namespace local {

	/// Creates and boots a host with \a args.
	template<typename THost, typename... TArgs>
	std::unique_ptr<THost> CreateAndBootHost(TArgs&&... args) {
		// create and boot the host
		auto pHost = std::make_unique<THost>(std::forward<TArgs>(args)...);
		try {
			pHost->boot();
		} catch (const std::exception& ex) {
			// log the exception and rethrow as a new exception in case the exception source is a dynamic plugin
			// (this avoids a potential crash in the calling code, which would occur if the host destructor unloads the source plugin)
			CATAPULT_LOG(fatal) << UNHANDLED_EXCEPTION_MESSAGE("boot");
			CATAPULT_THROW_RUNTIME_ERROR(ex.what());
		}

		return pHost;
	}

	/// Loads all plugins using \a bootstrapper.
	std::vector<plugins::PluginModule> LoadAllPlugins(extensions::ProcessBootstrapper& bootstrapper);
}}
