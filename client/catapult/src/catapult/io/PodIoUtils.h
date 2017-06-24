#pragma once
#include "catapult/utils/Traits.h"
#include <stdint.h>

namespace catapult { namespace io {

	/// Writes \a value into \a output.
	template<
		typename TIo,
		typename TValue,
		typename X = std::enable_if_t<utils::traits::is_pod<TValue>::value>
	>
	void Write(TIo& output, const TValue& value) {
		output.write({ reinterpret_cast<const uint8_t*>(&value), sizeof(TValue) });
	}

	/// Reads data of type \a TValue from \a input into \a value.
	template<
		typename TValue,
		typename TIo,
		typename X = std::enable_if_t<utils::traits::is_pod<TValue>::value>
	>
	void Read(TIo& input, TValue& value) {
		input.read({ reinterpret_cast<uint8_t*>(&value), sizeof(TValue) });
	}

	/// Reads data of type \a TValue from \a input.
	template<
		typename TValue,
		typename TIo,
		typename X = std::enable_if_t<utils::traits::is_pod<TValue>::value>
	>
	TValue Read(TIo& input) {
		TValue result;
		Read(input, result);
		return result;
	}
}}
