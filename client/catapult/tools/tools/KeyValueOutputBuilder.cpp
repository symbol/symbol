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
