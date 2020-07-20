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

#include "ConfigurationUtils.h"
#include <cctype>

namespace catapult { namespace utils {

	std::string GetIniPropertyName(const char* cppVariableName) {
		if (!cppVariableName || strlen(cppVariableName) < 2)
			CATAPULT_THROW_INVALID_ARGUMENT("cpp variable name must be at least two characters");

		auto firstChar = cppVariableName[0];
		if (!std::isalpha(firstChar))
			CATAPULT_THROW_INVALID_ARGUMENT("cpp variable name must start with a letter");

		// lowercase the first character
		return static_cast<char>(std::tolower(firstChar)) + std::string(&cppVariableName[1]);
	}

	void VerifyBagSizeExact(const ConfigurationBag& bag, size_t expectedSize) {
		if (expectedSize == bag.size())
			return;

		constexpr auto Error_Message = "configuration bag has unexpected number of properties (expected, actual)";
		CATAPULT_THROW_INVALID_ARGUMENT_2(Error_Message, expectedSize, bag.size());
	}

	ConfigurationBag ExtractSectionAsBag(const ConfigurationBag& bag, const char* section) {
		ConfigurationBag::ValuesContainer values;
		values.emplace("", bag.getAllOrdered<std::string>(section));
		return ConfigurationBag(std::move(values));
	}

	std::pair<std::unordered_set<std::string>, size_t> ExtractSectionAsUnorderedSet(const ConfigurationBag& bag, const char* section) {
		auto pair = ExtractSectionAsOrderedVector(bag, section);
		return std::make_pair(std::unordered_set<std::string>(pair.first.cbegin(), pair.first.cend()), pair.second);
	}

	std::pair<std::vector<std::string>, size_t> ExtractSectionAsOrderedVector(const ConfigurationBag& bag, const char* section) {
		auto keyValuePairs = bag.getAllOrdered<bool>(section);

		std::vector<std::string> enabledKeys;
		for (const auto& pair : keyValuePairs) {
			if (pair.second)
				enabledKeys.emplace_back(pair.first);
		}

		return std::make_pair(std::move(enabledKeys), keyValuePairs.size());
	}
}}
