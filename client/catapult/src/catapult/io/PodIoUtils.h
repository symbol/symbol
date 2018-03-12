#pragma once
#include "catapult/utils/BaseValue.h"
#include "catapult/utils/traits/Traits.h"
#include <array>
#include <stdint.h>

namespace catapult { namespace io {

	/// Writes base \a value into \a output.
	template<typename TIo, typename TValue, typename TTag, typename TBaseValue>
	void Write(TIo& output, const utils::BasicBaseValue<TValue, TTag, TBaseValue>& value) {
		output.write({ reinterpret_cast<const uint8_t*>(&value), sizeof(TValue) });
	}

	/// Writes \a buffer into \a output.
	template<typename TIo, size_t Array_Size>
	void Write(TIo& output, const std::array<uint8_t, Array_Size>& buffer) {
		output.write(buffer);
	}

	/// Writes \a value into \a output.
	template<typename TIo>
	void Write64(TIo& output, uint64_t value) {
		output.write({ reinterpret_cast<const uint8_t*>(&value), sizeof(uint64_t) });
	}

	/// Writes \a value into \a output.
	template<typename TIo>
	void Write32(TIo& output, uint32_t value) {
		output.write({ reinterpret_cast<const uint8_t*>(&value), sizeof(uint32_t) });
	}

	/// Writes \a value into \a output.
	template<typename TIo>
	void Write8(TIo& output, uint8_t value) {
		output.write({ reinterpret_cast<const uint8_t*>(&value), sizeof(uint8_t) });
	}

	/// Reads base \a value from \a input.
	template<typename TIo, typename TValue, typename TTag, typename TBaseValue>
	void Read(TIo& input, utils::BasicBaseValue<TValue, TTag, TBaseValue>& value) {
		input.read({ reinterpret_cast<uint8_t*>(&value), sizeof(TValue) });
	}

	/// Reads \a buffer from \a input.
	template<typename TIo, size_t Array_Size>
	void Read(TIo& input, std::array<uint8_t, Array_Size>& buffer) {
		input.read(buffer);
	}

	/// Reads \a value from \a input.
	template<typename TIo>
	auto Read64(TIo& input) {
		uint64_t result;
		input.read({ reinterpret_cast<uint8_t*>(&result), sizeof(uint64_t) });
		return result;
	}

	/// Reads \a value from \a input.
	template<typename TIo>
	auto Read32(TIo& input) {
		uint32_t result;
		input.read({ reinterpret_cast<uint8_t*>(&result), sizeof(uint32_t) });
		return result;
	}

	/// Reads \a value from \a input.
	template<typename TIo>
	auto Read8(TIo& input) {
		uint8_t result;
		input.read({ reinterpret_cast<uint8_t*>(&result), sizeof(uint8_t) });
		return result;
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
