/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
***
*** This file is part of Catapult.
***
*** Catapult is free software: you can redistribute it and/or modify
*** it under the terms of the GNU Lesser General Public License as published by
*** the Free Software Foundation, either version 3 of the License, or
*** (at your option) any later version.
***
*** Catapult is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*** GNU Lesser General Public License for more details.
***
*** You should have received a copy of the GNU Lesser General Public License
*** along with Catapult. If not, see <http://www.gnu.org/licenses/>.
**/

#include "ValidateConfiguration.h"
#include "LocalNodeConfiguration.h"
#include "catapult/crypto/KeyUtils.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/HexParser.h"

namespace catapult { namespace config {

#define CATAPULT_THROW_VALIDATION_ERROR(MESSAGE) CATAPULT_THROW_AND_LOG_0(utils::property_malformed_error, MESSAGE)

	namespace {
		void ValidateConfiguration(const UserConfiguration& config) {
			if (!crypto::IsValidKeyString(config.BootKey))
				CATAPULT_THROW_VALIDATION_ERROR("BootKey must be a valid private key");
		}

		void ValidateConfiguration(const model::BlockChainConfiguration& config) {
			if (2 * config.ImportanceGrouping <= config.MaxRollbackBlocks)
				CATAPULT_THROW_VALIDATION_ERROR("ImportanceGrouping must be greater than MaxRollbackBlocks / 2");
		}
	}

	void ValidateConfiguration(const LocalNodeConfiguration& config) {
		ValidateConfiguration(config.User);
		ValidateConfiguration(config.BlockChain);
	}

#undef CATAPULT_THROW_VALIDATION_ERROR
}}
