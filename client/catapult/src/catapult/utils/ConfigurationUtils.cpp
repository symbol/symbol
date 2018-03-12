#include "ConfigurationUtils.h"
#include <cctype>

namespace catapult { namespace utils {

	std::string GetIniPropertyName(const char* cppVariableName) {
		if (nullptr == cppVariableName || strlen(cppVariableName) < 2)
			CATAPULT_THROW_INVALID_ARGUMENT("cpp variable name must be at least two characters");

		auto firstChar = cppVariableName[0];
		if (!std::isalpha(firstChar))
			CATAPULT_THROW_INVALID_ARGUMENT("cpp variable name must start with a letter");

		// lowercase the first character
		return static_cast<char>(std::tolower(firstChar)) + std::string(&cppVariableName[1]);
	}

	void VerifyBagSizeLte(const ConfigurationBag& bag, size_t expectedSize) {
		if (bag.size() > expectedSize)
			CATAPULT_THROW_INVALID_ARGUMENT_1("configuration bag contains too many properties", bag.size());
	}

	ConfigurationBag ExtractSectionAsBag(const ConfigurationBag& bag, const char* section) {
		ConfigurationBag::ValuesContainer values;
		values.emplace("", bag.getAll<std::string>(section));
		return ConfigurationBag(std::move(values));
	}

	std::pair<std::unordered_set<std::string>, size_t> ExtractSectionAsUnorderedSet(const ConfigurationBag& bag, const char* section) {
		auto keyValuePairs = bag.getAll<bool>(section);

		std::unordered_set<std::string> enabledKeys;
		for (const auto& pair : keyValuePairs) {
			if (pair.second)
				enabledKeys.emplace(pair.first);
		}

		return std::make_pair(std::move(enabledKeys), keyValuePairs.size());
	}
}}
