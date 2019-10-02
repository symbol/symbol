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

#include "MongoPluginLoader.h"
#include "catapult/plugins/PluginExceptions.h"
#include "catapult/utils/Logging.h"
#include "catapult/preprocessor.h"
#include <boost/exception_ptr.hpp>

namespace catapult { namespace mongo {

	namespace {
		using RegisterMongoSubsystemFunc = void (*)(MongoPluginManager&);
	}

	ATTRIBUTE_CALLS_PLUGIN_API
	void LoadPluginByName(MongoPluginManager& manager, PluginModules& modules, const std::string& directory, const std::string& name) {
		modules.emplace_back(directory, name);
		auto registerMongoSubsystem = modules.back().symbol<RegisterMongoSubsystemFunc>("RegisterMongoSubsystem");

		CATAPULT_LOG(info) << "registering dynamic mongo plugin " << name;
		try {
			registerMongoSubsystem(manager);
		} catch (...) {
			// since the module will be unloaded after this function exits, throw a copy of the exception that
			// is not dependent on the (soon to be unloaded) module
			auto exInfo = boost::diagnostic_information(boost::current_exception());
			CATAPULT_THROW_AND_LOG_0(plugins::plugin_load_error, exInfo.c_str());
		}
	}
}}
