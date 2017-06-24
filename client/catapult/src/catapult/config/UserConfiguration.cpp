#include "UserConfiguration.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace config {

#define LOAD_PROPERTY(SECTION, NAME) utils::LoadIniProperty(bag, SECTION, #NAME, config.NAME)

	UserConfiguration UserConfiguration::Uninitialized() {
		return UserConfiguration();
	}

	UserConfiguration UserConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		UserConfiguration config;

#define LOAD_ACCOUNT_PROPERTY(NAME) LOAD_PROPERTY("account", NAME)

		LOAD_ACCOUNT_PROPERTY(BootKey);
		LOAD_ACCOUNT_PROPERTY(HarvestKey);
		LOAD_ACCOUNT_PROPERTY(IsAutoHarvestingEnabled);
		LOAD_ACCOUNT_PROPERTY(MaxUnlockedAccounts);

#undef LOAD_ACCOUNT_PROPERTY

#define LOAD_STORAGE_PROPERTY(NAME) LOAD_PROPERTY("storage", NAME)

		LOAD_STORAGE_PROPERTY(DataDirectory);
		LOAD_STORAGE_PROPERTY(PluginsDirectory);

#undef LOAD_STORAGE_PROPERTY

		utils::VerifyBagSizeLte(bag, 6);
		return config;
	}

#undef LOAD_PROPERTY
}}
