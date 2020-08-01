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

	/// Creates a ratio from \a numerator and \a denominator.
	template<typename T>
	double to_ratio(T numerator, T denominator) {
		return static_cast<double>(numerator) / static_cast<double>(denominator);
	}

	/// Converts a strongly typed enumeration \a value to its underlying integral value.
	template<typename TEnum>
	constexpr std::underlying_type_t<TEnum> to_underlying_type(TEnum value) {
		return static_cast<std::underlying_type_t<TEnum>>(value);
	}

	/// Converts \a value from one integral type (\a TSource) to another (\a TDest).
	/// This cast can only be used when data truncation is possible.
	/// \note Exception is thrown when data truncation is detected.
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
