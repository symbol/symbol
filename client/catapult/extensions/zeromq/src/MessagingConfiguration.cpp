#include "MessagingConfiguration.h"
#include "catapult/config/ConfigurationFileLoader.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace zeromq {

#define LOAD_PROPERTY(NAME) utils::LoadIniProperty(bag, "messaging", #NAME, config.NAME)

	MessagingConfiguration MessagingConfiguration::Uninitialized() {
		return MessagingConfiguration();
	}

	MessagingConfiguration MessagingConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		MessagingConfiguration config;

		LOAD_PROPERTY(SubscriberPort);

		utils::VerifyBagSizeLte(bag, 1);
		return config;
	}

#undef LOAD_PROPERTY

	MessagingConfiguration MessagingConfiguration::LoadFromPath(const boost::filesystem::path& resourcesPath) {
		return config::LoadIniConfiguration<MessagingConfiguration>(resourcesPath / "config-messaging.properties");
	}
}}
