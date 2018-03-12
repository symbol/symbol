#include "PtConfiguration.h"
#include "catapult/config/ConfigurationFileLoader.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace partialtransaction {

#define LOAD_PROPERTY(NAME) utils::LoadIniProperty(bag, "partialtransactions", #NAME, config.NAME)

	PtConfiguration PtConfiguration::Uninitialized() {
		return PtConfiguration();
	}

	PtConfiguration PtConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		PtConfiguration config;

		LOAD_PROPERTY(CacheMaxResponseSize);
		LOAD_PROPERTY(CacheMaxSize);

		utils::VerifyBagSizeLte(bag, 2);
		return config;
	}

#undef LOAD_PROPERTY

	PtConfiguration PtConfiguration::LoadFromPath(const boost::filesystem::path& resourcesPath) {
		return config::LoadIniConfiguration<PtConfiguration>(resourcesPath / "config-pt.properties");
	}
}}
