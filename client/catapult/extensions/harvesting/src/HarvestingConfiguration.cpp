#include "HarvestingConfiguration.h"
#include "catapult/config/ConfigurationFileLoader.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace harvesting {

#define LOAD_PROPERTY(SECTION, NAME) utils::LoadIniProperty(bag, SECTION, #NAME, config.NAME)

	HarvestingConfiguration HarvestingConfiguration::Uninitialized() {
		return HarvestingConfiguration();
	}

	HarvestingConfiguration HarvestingConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		HarvestingConfiguration config;

#define LOAD_HARVESTING_PROPERTY(NAME) LOAD_PROPERTY("harvesting", NAME)

		LOAD_HARVESTING_PROPERTY(HarvestKey);
		LOAD_HARVESTING_PROPERTY(IsAutoHarvestingEnabled);
		LOAD_HARVESTING_PROPERTY(MaxUnlockedAccounts);

#undef LOAD_HARVESTING_PROPERTY

		utils::VerifyBagSizeLte(bag, 3);
		return config;
	}

#undef LOAD_PROPERTY

	HarvestingConfiguration HarvestingConfiguration::LoadFromPath(const boost::filesystem::path& resourcesPath) {
		return config::LoadIniConfiguration<HarvestingConfiguration>(resourcesPath / "config-harvesting.properties");
	}
}}
