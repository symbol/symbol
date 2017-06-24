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
				sectionValues.emplace(item.first, item.second.get_value<std::string>());

			values.emplace(section.first, std::move(sectionValues));
		}

		return ConfigurationBag(std::move(values));
	}

	ConfigurationBag ConfigurationBag::FromPath(const std::string& path) {
		std::ifstream inputStream(path);
		return ConfigurationBag::FromStream(inputStream);
	}
}}
