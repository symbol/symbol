#include "TransferConfiguration.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace config {

	TransferConfiguration TransferConfiguration::Uninitialized() {
		return TransferConfiguration();
	}

	TransferConfiguration TransferConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		TransferConfiguration config;
		utils::LoadIniProperty(bag, "", "MaxMessageSize", config.MaxMessageSize);
		utils::VerifyBagSizeLte(bag, 1);
		return config;
	}
}}
