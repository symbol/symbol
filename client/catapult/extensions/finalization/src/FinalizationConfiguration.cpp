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

#include "FinalizationConfiguration.h"
#include "catapult/config/ConfigurationFileLoader.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace finalization {

#define LOAD_PROPERTY(NAME) utils::LoadIniProperty(bag, "finalization", #NAME, config.NAME)

	FinalizationConfiguration FinalizationConfiguration::Uninitialized() {
		return FinalizationConfiguration();
	}

	FinalizationConfiguration FinalizationConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		FinalizationConfiguration config;
		config.VotingSetGrouping = 0;

		LOAD_PROPERTY(EnableVoting);
		LOAD_PROPERTY(EnableRevoteOnBoot);

		LOAD_PROPERTY(Size);
		LOAD_PROPERTY(Threshold);
		LOAD_PROPERTY(StepDuration);

		LOAD_PROPERTY(ShortLivedCacheMessageDuration);
		LOAD_PROPERTY(MessageSynchronizationMaxResponseSize);

		LOAD_PROPERTY(MaxHashesPerPoint);
		LOAD_PROPERTY(PrevoteBlocksMultiple);
		LOAD_PROPERTY(VotingKeyDilution);

		utils::VerifyBagSizeExact(bag, 10);
		return config;
	}

#undef LOAD_PROPERTY

	FinalizationConfiguration FinalizationConfiguration::LoadFromPath(const boost::filesystem::path& resourcesPath) {
		return config::LoadIniConfiguration<FinalizationConfiguration>(resourcesPath / "config-finalization.properties");
	}
}}
