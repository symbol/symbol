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

#include "ConfigurationBag.h"
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace pt = boost::property_tree;

namespace catapult { namespace utils {

	ConfigurationBag ConfigurationBag::FromStream(std::istream& input) {
		pt::ptree properties;
		pt::read_ini(input, properties);

		ValuesContainer values;
		for (const auto& section : properties) {
			ValuesContainer::value_type::second_type sectionValues;
			for (const auto& item : section.second)
				sectionValues.emplace_back(item.first, item.second.get_value<std::string>());

			values.emplace(section.first, std::move(sectionValues));
		}

		return ConfigurationBag(std::move(values));
	}

	ConfigurationBag ConfigurationBag::FromPath(const std::string& path) {
		std::ifstream inputStream(path);
		return ConfigurationBag::FromStream(inputStream);
	}
}}
