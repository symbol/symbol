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

	/// Fills \a unresolvedAddress with random data.
	void FillWithRandomData(UnresolvedAddress& unresolvedAddress);

	/// Fills \a value with random data.
	template<typename TValue, typename TTag, typename TBaseValue>
	void FillWithRandomData(utils::BasicBaseValue<TValue, TTag, TBaseValue>& value) {
		test::FillWithRandomData({ reinterpret_cast<uint8_t*>(&value), sizeof(TValue) });
	}

	/// Generates random vector of \a count elements.
	template<typename T>
	std::vector<T> GenerateRandomDataVector(size_t count) {
		std::vector<T> vec(count);
		test::FillWithRandomData({ reinterpret_cast<uint8_t*>(vec.data()), count * sizeof(T) });
		return vec;
	}
}}
