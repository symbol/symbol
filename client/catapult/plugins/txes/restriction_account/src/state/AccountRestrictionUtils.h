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
#include <array>
#include <cstdint>
#include <vector>

namespace catapult { namespace state {

	/// Converts \a value to a vector.
	template<typename T>
	auto ToVector(T value) {
		std::vector<uint8_t> vec(sizeof(T));
		reinterpret_cast<T&>(vec[0]) = value;
		return vec;
	}

	/// Converts an array (\a value) to a vector.
	template<size_t N>
	auto ToVector(const std::array<uint8_t, N>& value) {
		return std::vector<uint8_t>(value.cbegin(), value.cend());
	}
}}
