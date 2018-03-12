#include "DatabaseConfiguration.h"
#include "catapult/config/ConfigurationFileLoader.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace mongo {

#define LOAD_PROPERTY(SECTION, NAME) utils::LoadIniProperty(bag, SECTION, #NAME, config.NAME)

	DatabaseConfiguration DatabaseConfiguration::Uninitialized() {
		return DatabaseConfiguration();
	}

	DatabaseConfiguration DatabaseConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		DatabaseConfiguration config;

#define LOAD_DB_PROPERTY(NAME) LOAD_PROPERTY("database", NAME)

		LOAD_DB_PROPERTY(DatabaseUri);
		LOAD_DB_PROPERTY(DatabaseName);
		LOAD_DB_PROPERTY(MaxWriterThreads);

#undef LOAD_DB_PROPERTY

		auto pluginsPair = utils::ExtractSectionAsUnorderedSet(bag, "plugins");
		config.Plugins = pluginsPair.first;

		utils::VerifyBagSizeLte(bag, 3 + pluginsPair.second);
		return config;
	}

#undef LOAD_PROPERTY

	DatabaseConfiguration DatabaseConfiguration::LoadFromPath(const boost::filesystem::path& resourcesPath) {
		return config::LoadIniConfiguration<DatabaseConfiguration>(resourcesPath / "config-database.properties");
	}
}}
