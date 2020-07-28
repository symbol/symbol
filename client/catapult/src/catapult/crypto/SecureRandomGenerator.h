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
#include <cstring>
#include <limits>
#include <stdint.h>

namespace catapult { namespace crypto {

	/// Cryptographically secure generator for random numbers.
	class SecureRandomGenerator {
	public:
		using result_type = uint64_t;

	public:
		/// Gets the mininmum generated value.
		static constexpr result_type min() {
			return std::numeric_limits<result_type>::min();
		}

		/// Gets the maximum generated value.
		static constexpr result_type max() {
			return std::numeric_limits<result_type>::max();
		}

	public:
		/// Generates a random value.
		result_type operator()();

		/// Generates \a count random bytes into \a pOut.
		void fill(uint8_t* pOut, size_t count);
	};
}}
