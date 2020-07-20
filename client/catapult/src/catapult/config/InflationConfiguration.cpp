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

#include "InflationConfiguration.h"
#include "catapult/utils/ConfigurationBag.h"
#include "catapult/utils/ConfigurationUtils.h"

namespace catapult { namespace config {

	namespace {
		constexpr const char* Section_Name = "inflation";
		constexpr const char* Expected_Prefix = "starting-at-height-";

		bool GetHeightFromKey(const std::string& key, Height& height) {
			if (0 != key.find(Expected_Prefix))
				return false;

			return utils::TryParseValue(key.substr(std::strlen(Expected_Prefix)), height);
		}
	}

	InflationConfiguration InflationConfiguration::Uninitialized() {
		return InflationConfiguration();
	}

	InflationConfiguration InflationConfiguration::LoadFromBag(const utils::ConfigurationBag& bag) {
		if (0 == bag.size())
			CATAPULT_THROW_AND_LOG_0(utils::property_not_found_error, "required inflation section is missing");

		InflationConfiguration config;
		auto lastHeight = Height();
		Height height;
		auto inflationMap = bag.getAllOrdered<uint64_t>(Section_Name);
		for (const auto& pair : inflationMap) {
			if (!GetHeightFromKey(pair.first, height)) {
				auto message = "property could not be parsed";
				CATAPULT_THROW_AND_LOG_2(utils::property_malformed_error, message, std::string(Section_Name), pair.first);
			}

			config.InflationCalculator.add(height, Amount(pair.second));
			lastHeight = height;
		}

		utils::VerifyBagSizeExact(bag, config.InflationCalculator.size());
		return config;
	}
}}
