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

#include "TimeSynchronizationConfiguration.h"
#include "catapult/config/ConfigurationFileLoader.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace timesync {

#define LOAD_PROPERTY(SECTION, NAME) utils::LoadIniProperty(bag, SECTION, #NAME, config.NAME)

	TimeSynchronizationConfiguration TimeSynchronizationConfiguration::Uninitialized() {
		return TimeSynchronizationConfiguration();
	}

	TimeSynchronizationConfiguration TimeSynchronizationConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		TimeSynchronizationConfiguration config;

#define LOAD_TIMESYNC_PROPERTY(NAME) LOAD_PROPERTY("timesynchronization", NAME)

		LOAD_TIMESYNC_PROPERTY(MaxNodes);
		LOAD_TIMESYNC_PROPERTY(MinImportance);

#undef LOAD_TIMESYNC_PROPERTY

		utils::VerifyBagSizeExact(bag, 2);
		return config;
	}

#undef LOAD_PROPERTY

	TimeSynchronizationConfiguration TimeSynchronizationConfiguration::LoadFromPath(const boost::filesystem::path& resourcesPath) {
		return config::LoadIniConfiguration<TimeSynchronizationConfiguration>(resourcesPath / "config-timesync.properties");
	}
}}
