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

#pragma once
#include "ConfigurationBag.h"
#include "catapult/functions.h"
#include <unordered_set>
#include <vector>

namespace catapult {
namespace utils {

	/// Gets the ini property name corresponding to the cpp variable name (\a cppVariableName).
	std::string GetIniPropertyName(const char* cppVariableName);

	/// Loads an ini property from \a bag into \a value given a section name (\a section) and a cpp variable name
	/// (\a cppVariableName).
	template <typename T>
	void LoadIniProperty(const ConfigurationBag& bag, const char* section, const char* cppVariableName, T& value) {
		value = bag.get<T>(ConfigurationKey(section, GetIniPropertyName(cppVariableName).c_str()));
	}

	/// Verifies that the number of properties in \a bag is exactly equal to \a expectedSize.
	void VerifyBagSizeExact(const ConfigurationBag& bag, size_t expectedSize);

	/// Extracts all \a section properties from \a bag into a new bag with a single section with a default (empty string) name.
	ConfigurationBag ExtractSectionAsBag(const ConfigurationBag& bag, const char* section);

	/// Extracts all \a section properties from \a bag into an unordered set.
	/// \note All section properties are expected to be boolean and only ones with \c true values will be included.
	std::pair<std::unordered_set<std::string>, size_t> ExtractSectionAsUnorderedSet(const ConfigurationBag& bag, const char* section);

	/// Extracts all \a section properties from \a bag into an ordered vector.
	/// \note All section properties are expected to be boolean and only ones with \c true values will be included.
	std::pair<std::vector<std::string>, size_t> ExtractSectionAsOrderedVector(const ConfigurationBag& bag, const char* section);

	/// Extracts all \a section properties from \a bag into an ordered and typed vector using \a valueParser to parse values.
	/// \note All section properties are expected to be boolean and only ones with \c true values will be included.
	template <typename TValue>
	std::pair<std::vector<TValue>, size_t> ExtractSectionKeysAsTypedVector(
		const ConfigurationBag& bag,
		const char* section,
		const predicate<const std::string&, TValue&>& valueParser) {
		auto valuesPair = ExtractSectionAsOrderedVector(bag, section);

		std::vector<TValue> values;
		for (const auto& str : valuesPair.first) {
			TValue value;
			if (!valueParser(str, value)) {
				auto message = "property could not be parsed";
				CATAPULT_THROW_AND_LOG_2(property_malformed_error, message, std::string(section), std::string(str));
			}

			values.push_back(value);
		}

		return std::make_pair(std::move(values), valuesPair.second);
	}
}
}
