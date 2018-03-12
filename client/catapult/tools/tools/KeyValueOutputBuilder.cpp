#include "KeyValueOutputBuilder.h"
#include <algorithm>

namespace catapult { namespace tools {

	void KeyValueOutputBuilder::add(const std::string& key, const std::string& value) {
		if (value.empty())
			return;

		m_keyValuePairs.emplace_back(key, value);
	}

	void KeyValueOutputBuilder::add(const std::string& key, const std::vector<std::string>& values, const std::string& separator) {
		add(key, Join(values, separator));
	}

	std::string KeyValueOutputBuilder::str(const std::string& prefix) const {
		auto keyWidth = maxKeySize();

		std::ostringstream out;
		for (const auto& pair : m_keyValuePairs)
			out << std::endl << prefix << std::setw(keyWidth) << pair.first << " : " << pair.second;

		return out.str();
	}

	int KeyValueOutputBuilder::maxKeySize() const {
		if (m_keyValuePairs.empty())
			return 0;

		auto iter = std::max_element(m_keyValuePairs.cbegin(), m_keyValuePairs.cend(), [](const auto& lhs, const auto& rhs) {
			return lhs.first.size() < rhs.first.size();
		});

		return static_cast<int>(iter->first.size());
	}
}}
