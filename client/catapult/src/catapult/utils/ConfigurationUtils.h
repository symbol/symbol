#pragma once
#include "ConfigurationBag.h"
#include "catapult/exceptions.h"
#include "catapult/preprocessor.h"
#include <cctype>

namespace catapult { namespace utils {

	/// Gets the ini property name corresponding to the cpp variable name (\a cppVariableName).
	CATAPULT_INLINE
	std::string GetIniPropertyName(const char* cppVariableName) {
		if (nullptr == cppVariableName || strlen(cppVariableName) < 2)
			CATAPULT_THROW_INVALID_ARGUMENT("cpp variable name must be at least two characters");

		auto firstChar = cppVariableName[0];
		if (!std::isalpha(firstChar))
			CATAPULT_THROW_INVALID_ARGUMENT("cpp variable name must start with a letter");

		// lowercase the first character
		return static_cast<char>(std::tolower(firstChar)) + std::string(&cppVariableName[1]);
	}

	/// Loads an ini property from \a bag into \a value given a section name (\a section) and a cpp variable name
	/// (\a cppVariableName).
	template<typename T>
	void LoadIniProperty(const ConfigurationBag& bag, const char* section, const char* cppVariableName, T& value) {
		value = bag.get<T>(ConfigurationKey(section, GetIniPropertyName(cppVariableName).c_str()));
	}

	/// Verifies that the number of properties in \a bag is no greater than \a expectedSize.
	CATAPULT_INLINE
	void VerifyBagSizeLte(const ConfigurationBag& bag, size_t expectedSize) {
		if (bag.size() > expectedSize)
			CATAPULT_THROW_INVALID_ARGUMENT_1("configuration bag contains too many properties", bag.size());
	}

	/// Extracts all \a section properties from \a bag into a new bag with a single section with a default (empty string) name.
	CATAPULT_INLINE
	ConfigurationBag ExtractSectionAsBag(const ConfigurationBag& bag, const char* section) {
		ConfigurationBag::ValuesContainer values;
		values.emplace("", bag.getAll<std::string>(section));
		return ConfigurationBag(std::move(values));
	}
}}
