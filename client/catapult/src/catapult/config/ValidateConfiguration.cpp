#include "ValidateConfiguration.h"
#include "LocalNodeConfiguration.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/HexParser.h"

namespace catapult { namespace config {

#define THROW_VALIDATION_ERROR(MESSAGE) CATAPULT_THROW_AND_LOG_0(utils::property_malformed_error, MESSAGE)

	namespace {
		bool IsValidKeyString(const std::string& str) {
			Key key;
			return utils::TryParseHexStringIntoContainer(str.data(), str.size(), key);
		}

		bool IsHarvestKeyValid(const UserConfiguration& config) {
			return IsValidKeyString(config.HarvestKey) || (!config.IsAutoHarvestingEnabled && config.HarvestKey.empty());
		}

		void ValidateConfiguration(const UserConfiguration& config) {
			if (!IsValidKeyString(config.BootKey))
				THROW_VALIDATION_ERROR("BootKey must be a valid private key");

			if (!IsHarvestKeyValid(config))
				THROW_VALIDATION_ERROR("HarvestKey must be a valid private key");
		}

		void ValidateConfiguration(const model::BlockChainConfiguration& config) {
			if (2 * config.ImportanceGrouping <= config.MaxRollbackBlocks)
				THROW_VALIDATION_ERROR("ImportanceGrouping must be greater than MaxRollbackBlocks / 2");
		}
	}

	void ValidateConfiguration(const LocalNodeConfiguration& config) {
		ValidateConfiguration(config.User);
		ValidateConfiguration(config.BlockChain);
	}

#undef THROW_VALIDATION_ERROR
}}
