#include "NamespaceConfiguration.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace config {

	NamespaceConfiguration NamespaceConfiguration::Uninitialized() {
		return NamespaceConfiguration();
	}

	NamespaceConfiguration NamespaceConfiguration::LoadFromBag(const utils::ConfigurationBag & bag) {
		NamespaceConfiguration config;

#define LOAD_PROPERTY(NAME) utils::LoadIniProperty(bag, "", #NAME, config.NAME)

		LOAD_PROPERTY(MaxNameSize);
		LOAD_PROPERTY(MaxNamespaceDuration);
		LOAD_PROPERTY(NamespaceGracePeriodDuration);
		LOAD_PROPERTY(ReservedRootNamespaceNames);

		LOAD_PROPERTY(NamespaceRentalFeeSinkPublicKey);
		LOAD_PROPERTY(RootNamespaceRentalFeePerBlock);
		LOAD_PROPERTY(ChildNamespaceRentalFee);

		LOAD_PROPERTY(MaxChildNamespaces);
		LOAD_PROPERTY(MaxMosaicsPerAccount);

		LOAD_PROPERTY(MaxMosaicDuration);

		LOAD_PROPERTY(IsMosaicLevyUpdateAllowed);
		LOAD_PROPERTY(MaxMosaicDivisibility);
		LOAD_PROPERTY(MaxMosaicDivisibleUnits);

		LOAD_PROPERTY(MosaicRentalFeeSinkPublicKey);
		LOAD_PROPERTY(MosaicRentalFee);

#undef LOAD_PROPERTY

		utils::VerifyBagSizeLte(bag, 15);
		return config;
	}
}}
