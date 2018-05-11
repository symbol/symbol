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

#include "NamespaceConfiguration.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace config {

	NamespaceConfiguration NamespaceConfiguration::Uninitialized() {
		return NamespaceConfiguration();
	}

	NamespaceConfiguration NamespaceConfiguration::LoadFromBag(const utils::ConfigurationBag & bag) {
		NamespaceConfiguration config;

#define LOAD_PROPERTY(NAME) utils::LoadIniProperty(bag, "", #NAME, config.NAME)

		LOAD_PROPERTY(MaxNameSize);
		LOAD_PROPERTY(MaxNamespaceDuration);
		LOAD_PROPERTY(NamespaceGracePeriodDuration);
		LOAD_PROPERTY(ReservedRootNamespaceNames);

		LOAD_PROPERTY(NamespaceRentalFeeSinkPublicKey);
		LOAD_PROPERTY(RootNamespaceRentalFeePerBlock);
		LOAD_PROPERTY(ChildNamespaceRentalFee);

		LOAD_PROPERTY(MaxChildNamespaces);
		LOAD_PROPERTY(MaxMosaicsPerAccount);

		LOAD_PROPERTY(MaxMosaicDuration);

		LOAD_PROPERTY(IsMosaicLevyUpdateAllowed);
		LOAD_PROPERTY(MaxMosaicDivisibility);
		LOAD_PROPERTY(MaxMosaicDivisibleUnits);

		LOAD_PROPERTY(MosaicRentalFeeSinkPublicKey);
		LOAD_PROPERTY(MosaicRentalFee);

#undef LOAD_PROPERTY

		utils::VerifyBagSizeLte(bag, 15);
		return config;
	}
}}
