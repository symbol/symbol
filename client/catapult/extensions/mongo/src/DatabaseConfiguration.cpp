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

#include "DatabaseConfiguration.h"
#include "catapult/config/ConfigurationFileLoader.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace mongo {

#define LOAD_PROPERTY(SECTION, NAME) utils::LoadIniProperty(bag, SECTION, #NAME, config.NAME)

	DatabaseConfiguration DatabaseConfiguration::Uninitialized() {
		return DatabaseConfiguration();
	}

	DatabaseConfiguration DatabaseConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		DatabaseConfiguration config;

#define LOAD_DB_PROPERTY(NAME) LOAD_PROPERTY("database", NAME)

		LOAD_DB_PROPERTY(DatabaseUri);
		LOAD_DB_PROPERTY(DatabaseName);
		LOAD_DB_PROPERTY(MaxWriterThreads);

#undef LOAD_DB_PROPERTY

		auto pluginsPair = utils::ExtractSectionAsUnorderedSet(bag, "plugins");
		config.Plugins = pluginsPair.first;

		utils::VerifyBagSizeExact(bag, 3 + pluginsPair.second);
		return config;
	}

#undef LOAD_PROPERTY

	DatabaseConfiguration DatabaseConfiguration::LoadFromPath(const boost::filesystem::path& resourcesPath) {
		return config::LoadIniConfiguration<DatabaseConfiguration>(resourcesPath / "config-database.properties");
	}
}}
