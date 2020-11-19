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
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

namespace catapult { namespace test {

	/// Converts an arbitrary pointer (\a pVoid) to a const void pointer.
	template<typename T>
	const void* AsVoidPointer(T* pVoid) {
		return reinterpret_cast<const void*>(const_cast<const T*>(pVoid));
	}

	/// Converts an arbitrary pointer (\a pData) to a const byte pointer.
	template<typename T>
	const uint8_t* AsBytePointer(const T* pData) {
		return reinterpret_cast<const uint8_t*>(pData);
	}

	/// Converts a binary buffer \a data to a hex string.
	std::string ToHexString(const std::vector<uint8_t>& data);

	/// Converts a hex string (\a hexString) to a vector.
	std::vector<uint8_t> HexStringToVector(const std::string& hexString);

	/// Converts \a value to a string and returns the result.
	template<typename T>
	std::string ToString(const T& value) {
		std::stringstream out;
		out << value;
		return out.str();
	}

	/// Converts an unordered set of \a pointers to values.
	template<typename T>
	auto PointersToValues(const std::unordered_set<T*>& pointers) {
		std::unordered_set<std::remove_cv_t<T>> values;
		for (const auto* pValue : pointers)
			values.insert(*pValue);

		return values;
	}
}}
