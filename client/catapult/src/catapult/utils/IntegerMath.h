#pragma once
#include "catapult/preprocessor.h"
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
		for (T i = 0u; i < GetNumBits<T>(); ++i) {
			if (static_cast<T>(1) == value)
				return i;

			value >>= 1;
		}

		return std::numeric_limits<T>::max();
	}

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
