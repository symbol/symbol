#include "MultisigConfiguration.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace config {

	MultisigConfiguration MultisigConfiguration::Uninitialized() {
		return MultisigConfiguration();
	}

	MultisigConfiguration MultisigConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		MultisigConfiguration config;

#define LOAD_PROPERTY(NAME) utils::LoadIniProperty(bag, "", #NAME, config.NAME)
		LOAD_PROPERTY(MaxMultisigDepth);
		LOAD_PROPERTY(MaxCosignersPerAccount);
		LOAD_PROPERTY(MaxCosignedAccountsPerAccount);
#undef LOAD_PROPERTY

		utils::VerifyBagSizeLte(bag, 3);
		return config;
	}
}}
