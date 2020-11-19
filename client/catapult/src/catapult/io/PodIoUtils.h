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
#include "catapult/utils/BaseValue.h"
#include "catapult/utils/traits/Traits.h"
#include <stdint.h>

namespace catapult { namespace io {

	/// Writes base \a value into \a output.
	template<typename TIo, typename TValue, typename TTag, typename TBaseValue>
	void Write(TIo& output, const utils::BasicBaseValue<TValue, TTag, TBaseValue>& value) {
		output.write({ reinterpret_cast<const uint8_t*>(&value), sizeof(TValue) });
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
	void Write16(TIo& output, uint16_t value) {
		output.write({ reinterpret_cast<const uint8_t*>(&value), sizeof(uint16_t) });
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

	/// Reads value from \a input.
	template<typename TIo>
	auto Read64(TIo& input) {
		uint64_t result;
		input.read({ reinterpret_cast<uint8_t*>(&result), sizeof(uint64_t) });
		return result;
	}

	/// Reads value from \a input.
	template<typename TIo>
	auto Read32(TIo& input) {
		uint32_t result;
		input.read({ reinterpret_cast<uint8_t*>(&result), sizeof(uint32_t) });
		return result;
	}

	/// Reads value from \a input.
	template<typename TIo>
	auto Read16(TIo& input) {
		uint16_t result;
		input.read({ reinterpret_cast<uint8_t*>(&result), sizeof(uint16_t) });
		return result;
	}

	/// Reads value from \a input.
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
		typename X = std::enable_if_t<utils::traits::is_pod_v<TValue>>
	>
	TValue Read(TIo& input) {
		TValue result;
		Read(input, result);
		return result;
	}
}}
