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
#include <limits>

namespace catapult { namespace test {

	/// Sets the specified \a value to the maximum value allowed by its type.
	template<typename T>
	void SetMaxValue(T& value) {
		value = std::numeric_limits<T>::max();
	}

	/// Creates a copy of \a data and XORs all its bytes.
	template<size_t N>
	std::array<uint8_t, N> CopyAndXorArray(const std::array<uint8_t, N>& data) {
		std::array<uint8_t, N> copy;
		for (auto i = 0u; i < N; ++i)
			copy[i] = data[i] ^ 0xFFu;

		return copy;
	}
}}
