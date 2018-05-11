/**
*** Copyright (c) 2016-present,
*** Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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
#include <array>
#include <sstream>
#include <string>
#include <vector>

namespace catapult { namespace test {

	/// Converts an arbitrary pointer (\a pVoid) to a const void pointer.
	template<typename T>
	const void* AsVoidPointer(T* pVoid) {
		return reinterpret_cast<const void*>(const_cast<const T*>(pVoid));
	}

	/// Converts a binary buffer pointed to by \a pData with size \a dataSize to a hex string.
	std::string ToHexString(const uint8_t* pData, size_t dataSize);

	/// Converts a binary buffer \a data to a hex string.
	std::string ToHexString(const std::vector<uint8_t>& data);

	/// Converts a binary buffer \a data to a hex string.
	template<size_t N>
	std::string ToHexString(const std::array<uint8_t, N>& data) {
		return ToHexString(data.data(), data.size());
	}

	/// Converts a hex string (\a hexString) to a vector.
	std::vector<uint8_t> ToVector(const std::string& hexString);

	/// Converts a hex string (\a hexString) to an array.
	template<size_t N>
	std::array<uint8_t, N> ToArray(const std::string& hexString) {
		std::array<uint8_t, N> array;
		auto vec = ToVector(hexString);
		std::copy(vec.cbegin(), vec.cend(), array.begin());
		return array;
	}

	/// Converts \a value to a string and returns the result.
	template<typename T>
	std::string ToString(const T& value) {
		std::stringstream out;
		out << value;
		return out.str();
	}

	/// Converts \a value to a string and returns the result.
	template<size_t N>
	std::string ToString(const std::array<uint8_t, N>& value) {
		return ToHexString(value);
	}
}}
