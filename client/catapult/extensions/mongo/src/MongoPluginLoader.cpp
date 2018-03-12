#include "MongoPluginLoader.h"
#include "catapult/plugins/PluginExceptions.h"
#include "catapult/utils/Logging.h"
#include <boost/exception_ptr.hpp>

namespace catapult { namespace mongo {

	namespace {
		using RegisterMongoSubsystemFunc = void (*)(MongoPluginManager&);
	}

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
