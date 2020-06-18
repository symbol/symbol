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

	// region random suppliers

	/// Generates a uint64_t random number.
	uint64_t Random();

	/// Generates a uint8_t random number.
	uint8_t RandomByte();

	// endregion

	// region fill with random data

	/// Fills a vector (\a vec) with random data.
	void FillWithRandomData(std::vector<uint8_t>& vec);

	/// Fills a buffer \a dataBuffer with random data.
	void FillWithRandomData(const MutableRawBuffer& dataBuffer);

	/// Fills \a unresolvedAddress with random data.
	void FillWithRandomData(UnresolvedAddress& unresolvedAddress);

	/// Fills \a value with random data.
	template<typename TValue, typename TTag, typename TBaseValue>
	void FillWithRandomData(utils::BasicBaseValue<TValue, TTag, TBaseValue>& value) {
		FillWithRandomData({ reinterpret_cast<uint8_t*>(&value), sizeof(TValue) });
	}

	/// Fills \a structure with random data.
	template<typename TStruct>
	void FillWithRandomData(TStruct& structure) {
		FillWithRandomData({ reinterpret_cast<uint8_t*>(&structure), sizeof(TStruct) });
	}

	// endregion

	// region generate random objects

	/// Generates a random base value.
	template<typename TBaseValue>
	TBaseValue GenerateRandomValue() {
		return TBaseValue(static_cast<typename TBaseValue::ValueType>(Random()));
	}

	/// Generates random array data.
	template<size_t N>
	std::array<uint8_t, N> GenerateRandomArray() {
		std::array<uint8_t, N> data;
		FillWithRandomData(data);
		return data;
	}

	/// Generates random array data.
	template<
		typename TArray,
		typename X = std::enable_if_t<utils::traits::is_template_specialization_v<TArray, utils::ByteArray>>>
	TArray GenerateRandomByteArray() {
		TArray data;
		FillWithRandomData(data);
		return data;
	}

	/// Generates random struct.
	template<
		typename TStruct,
		typename X = std::enable_if_t<!utils::traits::is_container_v<TStruct>>,
		typename Y = std::enable_if_t<!utils::traits::is_scalar_v<TStruct>>>
	TStruct GenerateRandomPackedStruct() {
		TStruct data;
		FillWithRandomData(data);
		return data;
	}

	/// Generates random string data of \a size.
	std::string GenerateRandomString(size_t size);

	/// Generates random hex string data of \a size.
	std::string GenerateRandomHexString(size_t size);

	/// Generates random vector data of \a size.
	std::vector<uint8_t> GenerateRandomVector(size_t size);

	/// Generates random vector of \a count elements.
	template<typename T>
	std::vector<T> GenerateRandomDataVector(size_t count) {
		std::vector<T> vec(count);
		FillWithRandomData({ reinterpret_cast<uint8_t*>(vec.data()), count * sizeof(T) });
		return vec;
	}

	// endregion

	// region vectors with unique random values

	/// Creates a value that is not contained in \a values.
	template<typename T>
	T CreateRandomUniqueValue(const std::vector<T>& values) {
		while (true) {
			T randomValue;
			FillWithRandomData({ reinterpret_cast<uint8_t*>(&randomValue), sizeof(T) });

			auto isFound = std::any_of(values.cbegin(), values.cend(), [&randomValue](const auto& value) {
				return value == randomValue;
			});
			if (!isFound)
				return randomValue;
		}
	}

	/// Generates random vector of \a count unique elements.
	template<typename T>
	std::vector<T> GenerateUniqueRandomDataVector(size_t count) {
		std::vector<T> vec;
		while (vec.size() < count)
			vec.push_back(CreateRandomUniqueValue(vec));

		return vec;
	}

	// endregion
}}
