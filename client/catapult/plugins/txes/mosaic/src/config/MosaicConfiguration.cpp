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

#include "MosaicConfiguration.h"
#include "catapult/model/Address.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

DEFINE_ADDRESS_CONFIGURATION_VALUE_SUPPORT

namespace catapult { namespace config {

	MosaicConfiguration MosaicConfiguration::Uninitialized() {
		return MosaicConfiguration();
	}

	MosaicConfiguration MosaicConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		MosaicConfiguration config;

#define LOAD_PROPERTY(NAME) utils::LoadIniProperty(bag, "", #NAME, config.NAME)

		LOAD_PROPERTY(MaxMosaicsPerAccount);
		LOAD_PROPERTY(MaxMosaicDuration);
		LOAD_PROPERTY(MaxMosaicDivisibility);

		LOAD_PROPERTY(MosaicRentalFeeSinkAddress);
		LOAD_PROPERTY(MosaicRentalFee);

#undef LOAD_PROPERTY

		utils::VerifyBagSizeLte(bag, 5);
		return config;
	}
}}
