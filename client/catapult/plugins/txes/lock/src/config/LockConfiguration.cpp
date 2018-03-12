#include "LockConfiguration.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace config {

	LockConfiguration LockConfiguration::Uninitialized() {
		return LockConfiguration();
	}

	LockConfiguration LockConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		LockConfiguration config;

#define LOAD_PROPERTY(NAME) utils::LoadIniProperty(bag, "", #NAME, config.NAME)
		LOAD_PROPERTY(LockedFundsPerAggregate);
		LOAD_PROPERTY(MaxHashLockDuration);
		LOAD_PROPERTY(MaxSecretLockDuration);
		LOAD_PROPERTY(MinProofSize);
		LOAD_PROPERTY(MaxProofSize);
#undef LOAD_PROPERTY

		utils::VerifyBagSizeLte(bag, 5);
		return config;
	}
}}
