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

#include "HarvestingConfiguration.h"
#include "catapult/config/ConfigurationFileLoader.h"
#include "catapult/model/Address.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

DEFINE_ADDRESS_CONFIGURATION_VALUE_SUPPORT_ALLOW_EMPTY(true)

namespace catapult { namespace harvesting {

#define LOAD_PROPERTY(SECTION, NAME) utils::LoadIniProperty(bag, SECTION, #NAME, config.NAME)

	HarvestingConfiguration HarvestingConfiguration::Uninitialized() {
		return HarvestingConfiguration();
	}

	HarvestingConfiguration HarvestingConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		HarvestingConfiguration config;

#define LOAD_HARVESTING_PROPERTY(NAME) LOAD_PROPERTY("harvesting", NAME)

		LOAD_HARVESTING_PROPERTY(HarvesterSigningPrivateKey);
		LOAD_HARVESTING_PROPERTY(HarvesterVrfPrivateKey);

		LOAD_HARVESTING_PROPERTY(EnableAutoHarvesting);
		LOAD_HARVESTING_PROPERTY(MaxUnlockedAccounts);
		LOAD_HARVESTING_PROPERTY(DelegatePrioritizationPolicy);
		LOAD_HARVESTING_PROPERTY(BeneficiaryAddress);

#undef LOAD_HARVESTING_PROPERTY

		utils::VerifyBagSizeExact(bag, 6);
		return config;
	}

#undef LOAD_PROPERTY

	HarvestingConfiguration HarvestingConfiguration::LoadFromPath(const boost::filesystem::path& resourcesPath) {
		return config::LoadIniConfiguration<HarvestingConfiguration>(resourcesPath / "config-harvesting.properties");
	}
}}
