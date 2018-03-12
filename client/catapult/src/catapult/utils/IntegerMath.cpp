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
}}
