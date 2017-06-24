#pragma once
#include "catapult/types.h"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <string>
#include <vector>

namespace catapult { namespace test {

	/// Generates a uint64_t random number.
	uint64_t Random();

	/// Generates a uint8_t random number.
	uint8_t RandomByte();

	/// Generates random array data.
	template<size_t N>
	std::array<uint8_t, N> GenerateRandomData() {
		std::array<uint8_t, N> data;
		std::generate_n(data.begin(), data.size(), RandomByte);
		return data;
	}

	/// Generates a random base value.
	template<typename T>
	T GenerateRandomValue() {
		return T(static_cast<typename T::ValueType>(Random()));
	}

	/// Generates random string data of \a size.
	std::string GenerateRandomString(size_t size);

	/// Generates random hex string data of \a size.
	std::string GenerateRandomHexString(size_t size);

	/// Generates random vector data of \a size.
	std::vector<uint8_t> GenerateRandomVector(size_t size);

	/// Fills a vector (\a vec) with random data.
	void FillWithRandomData(std::vector<uint8_t>& vec);

	/// Fills a buffer \a dataBuffer with random data.
	void FillWithRandomData(const MutableRawBuffer& dataBuffer);

	/// Generates random vector of \a count elements.
	template<typename T>
	std::vector<T> GenerateRandomDataVector(size_t count) {
		std::vector<T> vec(count);
		test::FillWithRandomData({ reinterpret_cast<uint8_t*>(vec.data()), count * sizeof(T) });
		return vec;
	}
}}
