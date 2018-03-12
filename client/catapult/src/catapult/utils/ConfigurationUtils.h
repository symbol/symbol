#pragma once
#include "ConfigurationBag.h"
#include <unordered_set>

namespace catapult { namespace utils {

	/// Gets the ini property name corresponding to the cpp variable name (\a cppVariableName).
	std::string GetIniPropertyName(const char* cppVariableName);

	/// Loads an ini property from \a bag into \a value given a section name (\a section) and a cpp variable name
	/// (\a cppVariableName).
	template<typename T>
	void LoadIniProperty(const ConfigurationBag& bag, const char* section, const char* cppVariableName, T& value) {
		value = bag.get<T>(ConfigurationKey(section, GetIniPropertyName(cppVariableName).c_str()));
	}

	/// Verifies that the number of properties in \a bag is no greater than \a expectedSize.
	void VerifyBagSizeLte(const ConfigurationBag& bag, size_t expectedSize);

	/// Extracts all \a section properties from \a bag into a new bag with a single section with a default (empty string) name.
	ConfigurationBag ExtractSectionAsBag(const ConfigurationBag& bag, const char* section);

	/// Extracts all \a section properties from \a bag into an unordered set.
	/// \note All section properties are expected to be boolean and only ones with \c true values will be included.
	std::pair<std::unordered_set<std::string>, size_t> ExtractSectionAsUnorderedSet(const ConfigurationBag& bag, const char* section);
}}
