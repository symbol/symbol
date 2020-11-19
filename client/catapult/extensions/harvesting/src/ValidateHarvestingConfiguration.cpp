/**
*** Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
*** Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
*** All rights reserved.
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

#include "ValidateHarvestingConfiguration.h"
#include "HarvestingConfiguration.h"
#include "catapult/crypto/KeyPair.h"
#include "catapult/utils/ConfigurationBag.h"

namespace catapult { namespace harvesting {

	namespace {
		bool IsHarvesterPrivateKeyValid(bool enableAutoHarvesting, const std::string& privateKey) {
			return crypto::Ed25519Utils::IsValidPrivateKeyString(privateKey) || (!enableAutoHarvesting && privateKey.empty());
		}
	}

	void ValidateHarvestingConfiguration(const HarvestingConfiguration& config) {
		if (!IsHarvesterPrivateKeyValid(config.EnableAutoHarvesting, config.HarvesterSigningPrivateKey))
			CATAPULT_THROW_AND_LOG_0(utils::property_malformed_error, "HarvesterSigningPrivateKey must be a valid private key");

		if (!IsHarvesterPrivateKeyValid(config.EnableAutoHarvesting, config.HarvesterVrfPrivateKey))
			CATAPULT_THROW_AND_LOG_0(utils::property_malformed_error, "HarvesterVrfPrivateKey must be a valid private key");
	}
}}
