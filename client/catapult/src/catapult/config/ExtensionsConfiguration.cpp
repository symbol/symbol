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

#include "ExtensionsConfiguration.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace config {

	ExtensionsConfiguration ExtensionsConfiguration::Uninitialized() {
		return ExtensionsConfiguration();
	}

	ExtensionsConfiguration ExtensionsConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		ExtensionsConfiguration config;

		if (0 == bag.size())
			CATAPULT_THROW_AND_LOG_0(utils::property_not_found_error, "required extensions section is missing");

		auto extensionsPair = utils::ExtractSectionAsOrderedVector(bag, "extensions");
		config.Names = extensionsPair.first;

		utils::VerifyBagSizeExact(bag, extensionsPair.second);
		return config;
	}
}}
