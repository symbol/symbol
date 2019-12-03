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

#include "IntegerMath.h"
#include "catapult/exceptions.h"

namespace catapult { namespace utils {

	// if 1 <= x < 2, y = log2(x), then 0 <= y < 1 so y has the binary expansion
	//
	// y = y1 * 2^-1 + y2 * 2^-2 + y3 * 2^-3 + ... = 2^-1 * (y1 + 2^-1 * (y2 + 2^-1 * (y3 + ...)))
	//
	// since x = 2^y, plugging in the expansion we get
	//
	// x = 2^(y1 * 2^-1 + y2 * 2^-2 + y3 * 2^-3 + ... = 2^-1 * (y1 + 2^-1 * (y2 + 2^-1 * (y3 + ...))))
	//
	// and thus
	//
	// x^2 = 2^y1 * 2^((y2 + 2^-1 * (y3 + 2^-1 * (y4 + ...))))
	// (*) note that y1 == 1 if and only if x^2 >= 2
	//
	// continuing that way we find
	//
	// x^(2^n) = 2^(2^(n-1) * y1 + 2^(n-2) * y2 + ... + 2^0 * yn) * 2^(2^-1 * (...))
	//
	// Therefore we finally get
	//
	// log2(x^(2^n)) = 2^(n-1) * y1 + 2^(n-2) * y2 + ... + 2^0 * yn
	//
	// with the observation (*) we can extract the coefficients y1, ..., yn
	uint64_t Log2TimesPowerOfTwo(uint64_t value, uint64_t n) {
		if (0 == value)
			CATAPULT_THROW_INVALID_ARGUMENT("log2(0) is not defined");

		constexpr uint64_t Precision = 31;
		constexpr uint64_t Power_Of_Two = 1ull << Precision;
		auto y = Precision;
		auto x = value;

		// normalize x in [1, 2)
		while (x < Power_Of_Two) {
			x <<= 1;
			--y;
		}

		// iteratively square x and extract one coefficient each round
		for (auto i = 0u; i < n; ++i) {
			x = (x * x) >> Precision;
			y <<= 1;
			if (x >= 2 * Power_Of_Two) {
				x >>= 1;
				y += 1;
			}
		}

		return y;
	}

	// compute 2^value in s15.16 fixed-point arithmetic, -15 < value < 15
	// note that the accuracy gets worse as value gets smaller, therefore it should only be used for -5 < value < 15
	uint32_t FixedPointPowerOfTwo(int32_t value) {
		// split value = i + f, such that f in (-0.5, 0.5]
		auto i = (value + 0x8000) & ~0xFFFF;
		auto f = value - i;
		auto s = ((15 << 16) - i) / 65'536;

		// minimax approximation for 2^f on (-0.5, 0.5]
		// 5.5171669058037949e-2 (approx. 0x00000E20 / 2^16)
		// 2.4261112219321804e-1 (approx. 0x3E1CC333 / 2^32)
		// 6.9326098546062365e-1 (approx. 0x58BD46A6 / 2^31)
		// 9.9992807353939517e-1 (approx. 0x7FFDE4A3 / 2^31)
		int64_t r = 0x00000E20;
		r = (r * f + 0x3E1CC333) / 131'072;
		r = (r * f + 0x58BD46A6) / 65'536;
		r = r * f + 0x7FFDE4A3;
		return static_cast<uint32_t>(r) >> s;
	}
}}
