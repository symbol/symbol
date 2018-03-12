#pragma once
#include "catapult/preprocessor.h"
#include <cstdint>
#include <limits>
#include <type_traits>

namespace catapult { namespace utils {

	/// Gets the number of bits in the specified type.
	template<typename T, typename X = typename std::enable_if<std::is_integral<T>::value>::type>
	constexpr T GetNumBits() {
		return static_cast<T>(8u * sizeof(T));
	}

	/// Calculates log2(\a value).
	template<typename T, typename X = typename std::enable_if<std::is_unsigned<T>::value>::type>
	CPP14_CONSTEXPR T Log2(T value) {
#ifdef _MSC_VER
		unsigned long result;
		if (!_BitScanReverse(&result, value))
			return std::numeric_limits<T>::max();

		return static_cast<T>(result);
#else
		if (!value)
			return std::numeric_limits<T>::max();

		return static_cast<T>(63 - __builtin_clzll(value));
#endif
	}

	/// Calculates log2(\a value^(2^\a n)).
	uint64_t Log2TimesPowerOfTwo(uint64_t value, uint64_t n);

	/// Calculates 2^(\a value).
	template<typename T, typename X = typename std::enable_if<std::is_unsigned<T>::value>::type>
	constexpr T Pow2(T value) {
		return value >= GetNumBits<T>() ? 0 : static_cast<T>(static_cast<T>(1) << value);
	}

	/// Divides \a value by \a divisor and returns the remainder.
	template<typename T, typename X = typename std::enable_if<std::is_unsigned<T>::value>::type>
	CPP14_CONSTEXPR T DivideAndGetRemainder(T& value, T divisor) {
		auto remainder = static_cast<T>(value % divisor);
		value /= divisor;
		return remainder;
	}
}}
