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
#include "catapult/types.h"
#include "tests/test/nodeps/Conversions.h"
#include <string>

namespace catapult { namespace test {

	/// String-based key wrapper for test purposes.
	class StringKey {
	public:
		/// Creates a string key around \a data.
		StringKey(const std::string& data) : m_data(data)
		{}

		/// Creates a string key around \a data.
		StringKey(const char* data) : m_data(data)
		{}

	public:
		/// Gets a pointer to data.
		auto data() const {
			return m_data.data();
		}

		/// Gets the size of data.
		auto size() const {
			return m_data.size();
		}

		/// Gets the data.
		const auto& str() const {
			return m_data;
		}

	public:
		/// Returns \c true if this is less than \a rhs.
		bool operator<(const StringKey& rhs) const {
			return m_data < rhs.m_data;
		}

	private:
		std::string m_data;
	};

	/// Serializes string-based \a key.
	inline RawBuffer SerializeKey(const StringKey& key) {
		return { AsBytePointer(key.data()), key.size() };
	}
}}
