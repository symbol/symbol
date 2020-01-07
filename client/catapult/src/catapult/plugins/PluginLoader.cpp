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

#include "PluginLoader.h"
#include "PluginExceptions.h"
#include "PluginManager.h"
#include "catapult/utils/Logging.h"
#include "catapult/functions.h"
#include <boost/exception_ptr.hpp>
#include <unordered_map>

namespace catapult { namespace plugins {

	namespace {
		void LoadPlugin(PluginManager& manager, const PluginModule& module, const char* symbolName) {
			auto registerSubsystem = module.symbol<decltype(::RegisterSubsystem)*>(symbolName);

			try {
				registerSubsystem(manager);
			} catch (...) {
				// since the module will be unloaded after this function exits, throw a copy of the exception that
				// is not dependent on the (soon to be unloaded) module
				auto exInfo = boost::diagnostic_information(boost::current_exception());
				CATAPULT_THROW_AND_LOG_0(plugin_load_error, exInfo.c_str());
			}
		}
	}

	void LoadPluginByName(PluginManager& manager, PluginModules& modules, const std::string& directory, const std::string& name) {
		CATAPULT_LOG(info) << "registering dynamic plugin " << name;

		modules.emplace_back(directory, name);
		LoadPlugin(manager, modules.back(), "RegisterSubsystem");
	}
}}
