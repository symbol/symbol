#include "AggregateConfiguration.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace config {

	AggregateConfiguration AggregateConfiguration::Uninitialized() {
		return AggregateConfiguration();
	}

	AggregateConfiguration AggregateConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		AggregateConfiguration config;

#define LOAD_PROPERTY(NAME) utils::LoadIniProperty(bag, "", #NAME, config.NAME)
		LOAD_PROPERTY(MaxTransactionsPerAggregate);
		LOAD_PROPERTY(MaxCosignaturesPerAggregate);
		LOAD_PROPERTY(EnableStrictCosignatureCheck);
#undef LOAD_PROPERTY

		utils::VerifyBagSizeLte(bag, 3);
		return config;
	}
}}
