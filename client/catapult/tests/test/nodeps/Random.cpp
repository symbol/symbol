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

#include "Random.h"
#include "catapult/utils/RandomGenerator.h"

namespace catapult { namespace test {

	namespace {
		template<typename T>
		void RandomFill(T& container) {
			std::generate_n(container.begin(), container.size(), []() {
				return static_cast<typename T::value_type>(RandomByte());
			});
		}

		template<typename T>
		T GenerateRandomContainer(size_t size) {
			T container;
			container.resize(size);
			RandomFill(container);
			return container;
		}
	}

	// region random suppliers

	uint64_t Random() {
		return utils::LowEntropyRandomGenerator()();
	}

	uint8_t RandomByte() {
		return static_cast<uint8_t>(Random());
	}

	// endregion

	// region fill with random data

	void FillWithRandomData(std::vector<uint8_t>& vec) {
		RandomFill(vec);
	}

	void FillWithRandomData(const MutableRawBuffer& dataBuffer) {
		std::generate_n(dataBuffer.pData, dataBuffer.Size, RandomByte);
	}

	void FillWithRandomData(UnresolvedAddress& unresolvedAddress) {
		FillWithRandomData({ reinterpret_cast<uint8_t*>(unresolvedAddress.data()), unresolvedAddress.size() });
	}

	// endregion

	// region generate random objects

	std::string GenerateRandomString(size_t size) {
		return GenerateRandomContainer<std::string>(size);
	}

	std::string GenerateRandomHexString(size_t size) {
		std::string str;
		str.resize(size);
		std::generate_n(str.begin(), str.size(), []() {
			auto value = Random() % 16;
			return static_cast<char>(value < 10 ? (value + '0') : (value - 10 + 'a'));
		});
		return str;
	}

	std::vector<uint8_t> GenerateRandomVector(size_t size) {
		return GenerateRandomDataVector<uint8_t>(size);
	}

	// endregion
}}
