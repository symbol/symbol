#pragma once
#include "catapult/exceptions.h"
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace catapult { namespace utils {

	/// Coerces a reference (\a ref) to a const reference.
	template<typename T>
	constexpr const T& as_const(T& ref) {
		return ref;
	}

	/// Converts a strongly typed enumeration \a value to its underlying integral value.
	template<typename TEnum>
	constexpr std::underlying_type_t<TEnum> to_underlying_type(TEnum value) {
		return static_cast<std::underlying_type_t<TEnum>>(value);
	}

	/// Converts \a value from one integral type (\a TSource) to another (\a TDest).
	/// This cast can only be used when data truncation is possible.
	/// An exception is thrown if data truncation is detected.
	template<typename TSource, typename TDest>
	TDest checked_cast(TSource value) {
		using dest_limits = std::numeric_limits<TDest>;
		using source_limits = std::numeric_limits<TSource>;
		static_assert(
			source_limits::min() < dest_limits::min() || source_limits::max() > dest_limits::max(),
			"checked_cast can only be used when data truncation is possible");

		if (value < dest_limits::min() || value > dest_limits::max())
			CATAPULT_THROW_RUNTIME_ERROR_1("checked_cast detected data truncation", value);

		return static_cast<TDest>(value);
	}
}}
