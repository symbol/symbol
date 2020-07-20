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

#include "UserConfiguration.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace config {

#define LOAD_PROPERTY(SECTION, NAME) utils::LoadIniProperty(bag, SECTION, #NAME, config.NAME)

	UserConfiguration UserConfiguration::Uninitialized() {
		return UserConfiguration();
	}

	UserConfiguration UserConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		UserConfiguration config;

#define LOAD_ACCOUNT_PROPERTY(NAME) LOAD_PROPERTY("account", NAME)

		LOAD_ACCOUNT_PROPERTY(EnableDelegatedHarvestersAutoDetection);

#undef LOAD_ACCOUNT_PROPERTY

#define LOAD_STORAGE_PROPERTY(NAME) LOAD_PROPERTY("storage", NAME)

		LOAD_STORAGE_PROPERTY(DataDirectory);
		LOAD_STORAGE_PROPERTY(CertificateDirectory);
		LOAD_STORAGE_PROPERTY(PluginsDirectory);

#undef LOAD_STORAGE_PROPERTY

		utils::VerifyBagSizeExact(bag, 4);
		return config;
	}

#undef LOAD_PROPERTY
}}
