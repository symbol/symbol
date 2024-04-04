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
#include "Hashers.h"
#include "Logging.h"
#include "catapult/types.h"
#include <array>
#include <unordered_set>

namespace catapult {
	namespace utils {
		class BlockSpan;
		class FileSize;
		class TimeSpan;
	}
}

namespace catapult { namespace utils {

	/// Tries to parse \a str into a log level (\a parsedValue).
	bool TryParseValue(const std::string& str, LogLevel& parsedValue);

	/// Tries to parse \a str into a log sink type (\a parsedValue).
	bool TryParseValue(const std::string& str, LogSinkType& parsedValue);

	/// Tries to parse \a str into a log color mode (\a parsedValue).
	bool TryParseValue(const std::string& str, LogColorMode& parsedValue);

	/// Tries to parse \a str into a boolean (\a parsedValue).
	bool TryParseValue(const std::string& str, bool& parsedValue);

	/// Tries to parse \a str into a uint8_t (\a parsedValue).
	bool TryParseValue(const std::string& str, uint8_t& parsedValue);

	/// Tries to parse \a str into a uint16_t (\a parsedValue).
	bool TryParseValue(const std::string& str, uint16_t& parsedValue);

	/// Tries to parse \a str into a uint32_t (\a parsedValue).
	bool TryParseValue(const std::string& str, uint32_t& parsedValue);

	/// Tries to parse \a str into a uint64_t (\a parsedValue).
	bool TryParseValue(const std::string& str, uint64_t& parsedValue);

	/// Tries to parse \a str into an Amount (\a parsedValue).
	bool TryParseValue(const std::string& str, Amount& parsedValue);

	/// Tries to parse \a str into a BlockFeeMultiplier (\a parsedValue).
	bool TryParseValue(const std::string& str, BlockFeeMultiplier& parsedValue);

	/// Tries to parse \a str into a Height (\a parsedValue).
	bool TryParseValue(const std::string& str, Height& parsedValue);

	/// Tries to parse \a str into an Importance (\a parsedValue).
	bool TryParseValue(const std::string& str, Importance& parsedValue);

	/// Tries to parse \a str into a FinalizationEpoch (\a parsedValue).
	bool TryParseValue(const std::string& str, FinalizationEpoch& parsedValue);

	/// Tries to parse \a str into a MosaicId (\a parsedValue).
	bool TryParseValue(const std::string& str, MosaicId& parsedValue);

	/// Tries to parse \a str into a TimeSpan (\a parsedValue).
	bool TryParseValue(const std::string& str, TimeSpan& parsedValue);

	/// Tries to parse \a str into a BlockSpan (\a parsedValue).
	bool TryParseValue(const std::string& str, BlockSpan& parsedValue);

	/// Tries to parse \a str into a FileSize (\a parsedValue).
	bool TryParseValue(const std::string& str, FileSize& parsedValue);

	/// Tries to parse \a str into a Key (\a parsedValue).
	bool TryParseValue(const std::string& str, Key& parsedValue);

	/// Tries to parse \a str into a Hash256 (\a parsedValue).
	bool TryParseValue(const std::string& str, Hash256& parsedValue);

	/// Tries to parse \a str into a GenerationHashSeed (\a parsedValue).
	bool TryParseValue(const std::string& str, GenerationHashSeed& parsedValue);

	/// Tries to parse \a str into a string (\a parsedValue).
	/// \note This function just copies \a str into \a parsedValue.
	bool TryParseValue(const std::string& str, std::string& parsedValue);

	/// Tries to parse \a str into a set of strings (\a parsedValue).
	/// \note \a str is expected to be comma separated
	bool TryParseValue(const std::string& str, std::unordered_set<std::string>& parsedValue);

	/// Tries to parse \a str into a set of heights (\a parsedValue).
	/// \note \a str is expected to be comma separated
	bool TryParseValue(const std::string& str, std::unordered_set<Height, BaseValueHasher<Height>>& parsedValue);

	/// Tries to parse \a str into an enum value (\a parsedValue) given a mapping of strings to values (\a stringToValueMapping).
	template<typename T, size_t N>
	bool TryParseEnumValue(const std::array<std::pair<const char*, T>, N>& stringToValueMapping, const std::string& str, T& parsedValue) {
		auto iter = std::find_if(stringToValueMapping.cbegin(), stringToValueMapping.cend(), [&str](const auto& pair) {
			return pair.first == str;
		});

		if (stringToValueMapping.cend() == iter)
			return false;

		parsedValue = iter->second;
		return true;
	}

	/// Tries to parse \a str into a bitwise enum value (\a parsedValue) given a mapping of strings to values (\a stringToValueMapping).
	template<typename T, size_t N>
	bool TryParseBitwiseEnumValue(
			const std::array<std::pair<const char*, T>, N>& stringToValueMapping,
			const std::string& str,
			T& parsedValues) {
		std::unordered_set<std::string> parts;
		if (!TryParseValue(str, parts))
			return false;

		T values = static_cast<T>(0);
		for (const auto& part : parts) {
			T value;
			if (!TryParseEnumValue(stringToValueMapping, part, value))
				return false;

			values |= value;
		}

		parsedValues = values;
		return true;
	}
}}
