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
