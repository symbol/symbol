#include "ValidateHarvestingConfiguration.h"
#include "HarvestingConfiguration.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/HexParser.h"

namespace catapult { namespace harvesting {

#define THROW_VALIDATION_ERROR(MESSAGE) CATAPULT_THROW_AND_LOG_0(utils::property_malformed_error, MESSAGE)

	namespace {
		bool IsHarvestKeyValid(const HarvestingConfiguration& config) {
			return crypto::IsValidKeyString(config.HarvestKey) || (!config.IsAutoHarvestingEnabled && config.HarvestKey.empty());
		}
	}

	void ValidateHarvestingConfiguration(const HarvestingConfiguration& config) {
		if (!IsHarvestKeyValid(config))
			THROW_VALIDATION_ERROR("HarvestKey must be a valid private key");
	}

#undef THROW_VALIDATION_ERROR
}}
