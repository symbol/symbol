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
#include "catapult/utils/HexFormatter.h"
#include <sstream>
#include <vector>

namespace catapult { namespace tools {

	/// Builder for aggregating and formatting key value pairs for output.
	/// \note Empty values will be ignored and not output.
	class KeyValueOutputBuilder {
	public:
		/// Adds \a key with a string \a value.
		void add(const std::string& key, const std::string& value);

		/// Adds \a key with multiple \a values that should be joined with \a separator.
		void add(const std::string& key, const std::vector<std::string>& values, const std::string& separator);

		/// Adds \a key with a hex-formatted array \a value.
		template<size_t N>
		void add(const std::string& key, const std::array<uint8_t, N>& value) {
			std::ostringstream out;
			out << utils::HexFormat(value);
			add(key, out.str());
		}

		/// Adds \a key with a default formatted \a value.
		template<typename T>
		void add(const std::string& key, const T& value) {
			std::ostringstream out;
			out << value;
			add(key, out.str());
		}

	public:
		/// Gets a string composed of all formatted key value pairs.
		/// Each pair is on its own line and prefixed with \a prefix.
		std::string str(const std::string& prefix) const;

	private:
		int maxKeySize() const;

	private:
		std::vector<std::pair<std::string, std::string>> m_keyValuePairs;
	};

	/// Joins all \a values with \a separator.
	template<typename TValue>
	std::string Join(const std::vector<TValue>& values, const std::string& separator) {
		std::ostringstream out;
		for (auto i = 0u; i < values.size(); ++i) {
			out << values[i];

			if (values.size() != i + 1)
				out << separator;
		}

		return out.str();
	}
}}
