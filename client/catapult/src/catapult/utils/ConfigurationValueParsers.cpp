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

#include "ConfigurationValueParsers.h"
#include "BlockSpan.h"
#include "FileSize.h"
#include "HexParser.h"
#include "TimeSpan.h"

namespace catapult { namespace utils {

	// region enums / bool

	namespace {
		const std::array<std::pair<const char*, LogLevel>, 9> String_To_LogLevel_Pairs{{
			{ std::make_pair("Trace", LogLevel::trace) },
			{ std::make_pair("Debug", LogLevel::debug) },
			{ std::make_pair("Info", LogLevel::info) },
			{ std::make_pair("Important", LogLevel::important) },
			{ std::make_pair("Warning", LogLevel::warning) },
			{ std::make_pair("Error", LogLevel::error) },
			{ std::make_pair("Fatal", LogLevel::fatal) },
			{ std::make_pair("Min", LogLevel::min) },
			{ std::make_pair("Max", LogLevel::max) }
		}};

		const std::array<std::pair<const char*, LogSinkType>, 2> String_To_LogSinkType_Pairs{{
			{ std::make_pair("Sync", LogSinkType::Sync) },
			{ std::make_pair("Async", LogSinkType::Async) }
		}};

		const std::array<std::pair<const char*, LogColorMode>, 3> String_To_LogColorMode_Pairs{{
			{ std::make_pair("Ansi", LogColorMode::Ansi) },
			{ std::make_pair("AnsiBold", LogColorMode::AnsiBold) },
			{ std::make_pair("None", LogColorMode::None) }
		}};

		const std::array<std::pair<const char*, bool>, 2> String_To_Boolean_Pairs{{
			{ std::make_pair("true", true) },
			{ std::make_pair("false", false) }
		}};
	}

	bool TryParseValue(const std::string& str, LogLevel& parsedValue) {
		return TryParseEnumValue(String_To_LogLevel_Pairs, str, parsedValue);
	}

	bool TryParseValue(const std::string& str, LogSinkType& parsedValue) {
		return TryParseEnumValue(String_To_LogSinkType_Pairs, str, parsedValue);
	}

	bool TryParseValue(const std::string& str, LogColorMode& parsedValue) {
		return TryParseEnumValue(String_To_LogColorMode_Pairs, str, parsedValue);
	}

	bool TryParseValue(const std::string& str, bool& parsedValue) {
		return TryParseEnumValue(String_To_Boolean_Pairs, str, parsedValue);
	}

	// endregion

	// region int

	namespace {
		struct DecimalTraits {
			static constexpr uint8_t Base = 10;

			static bool IsDigit(char ch) {
				return ch >= '0' && ch <= '9';
			}

			static uint8_t GetByteValue(char ch) {
				return static_cast<uint8_t>(ch - '0');
			}
		};

		struct HexTraits {
			static constexpr uint8_t Base = 16;

			static bool IsDigit(char ch) {
				return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F');
			}

			static uint8_t GetByteValue(char ch) {
				return static_cast<uint8_t>(ch <= '9' ? ch - '0' : ch - 'A' + 10);
			}
		};

		template<typename TTraits, typename T>
		bool TryParseUnsignedIntValue(const std::string& str, T& parsedValue) {
			constexpr char Digit_Separator = '\'';
			if (str.empty())
				return false;

			T result = 0;
			bool isLastCharSeparator = true;
			for (auto ch : str) {
				// only support valid digits and non-consecutive separators
				bool isCharSeparator = Digit_Separator == ch;
				bool isDigit = TTraits::IsDigit(ch);
				bool isValidChar = isDigit || (isCharSeparator && !isLastCharSeparator);
				if (!isValidChar)
					return false;

				isLastCharSeparator = isCharSeparator;
				if (isLastCharSeparator)
					continue;

				// check for overflow
				auto digit = TTraits::GetByteValue(ch);
				if (static_cast<T>(result * TTraits::Base + digit) < result)
					return false;

				result = static_cast<T>(result * TTraits::Base + digit);
			}

			if (isLastCharSeparator)
				return false;

			parsedValue = result;
			return true;
		}

		template<typename T>
		bool TryParseUnsignedIntDecimalValue(const std::string& str, T& parsedValue) {
			return TryParseUnsignedIntValue<DecimalTraits>(str, parsedValue);
		}

		template<typename T>
		bool TryParseUnsignedIntHexValue(const std::string& str, T& parsedValue) {
			if (str.size() < 2 || 0 != std::memcmp("0x", str.data(), 2))
				return false;

			return TryParseUnsignedIntValue<HexTraits>(str.substr(2), parsedValue);
		}
	}

	bool TryParseValue(const std::string& str, uint8_t& parsedValue) {
		return TryParseUnsignedIntDecimalValue(str, parsedValue);
	}

	bool TryParseValue(const std::string& str, uint16_t& parsedValue) {
		return TryParseUnsignedIntDecimalValue(str, parsedValue);
	}

	bool TryParseValue(const std::string& str, uint32_t& parsedValue) {
		return TryParseUnsignedIntDecimalValue(str, parsedValue);
	}

	bool TryParseValue(const std::string& str, uint64_t& parsedValue) {
		return TryParseUnsignedIntDecimalValue(str, parsedValue);
	}

	// endregion

	// region custom types

	namespace {
		template<typename TNumeric, typename T, typename TFactory>
		bool TryParseCustomUnsignedIntDecimalValue(TFactory factory, const std::string& str, T& parsedValue) {
			TNumeric raw;
			if (!TryParseValue(str, raw))
				return false;

			parsedValue = factory(raw);
			return true;
		}

		template<typename TNumeric, typename T, typename TFactory>
		bool TryParseCustomUnsignedIntHexValue(TFactory factory, const std::string& str, T& parsedValue) {
			TNumeric raw;
			if (!TryParseUnsignedIntHexValue(str, raw))
				return false;

			parsedValue = factory(raw);
			return true;
		}
	}

	bool TryParseValue(const std::string& str, Amount& parsedValue) {
		return TryParseCustomUnsignedIntDecimalValue<Amount::ValueType>([](auto raw) { return Amount(raw); }, str, parsedValue);
	}

	bool TryParseValue(const std::string& str, BlockFeeMultiplier& parsedValue) {
		auto factory = [](auto raw) { return BlockFeeMultiplier(raw); };
		return TryParseCustomUnsignedIntDecimalValue<BlockFeeMultiplier::ValueType>(factory, str, parsedValue);
	}

	bool TryParseValue(const std::string& str, Height& parsedValue) {
		return TryParseCustomUnsignedIntDecimalValue<Height::ValueType>([](auto raw) { return Height(raw); }, str, parsedValue);
	}

	bool TryParseValue(const std::string& str, Importance& parsedValue) {
		return TryParseCustomUnsignedIntDecimalValue<Importance::ValueType>([](auto raw) { return Importance(raw); }, str, parsedValue);
	}

	bool TryParseValue(const std::string& str, FinalizationEpoch& parsedValue) {
		return TryParseCustomUnsignedIntDecimalValue<FinalizationEpoch::ValueType>(
				[](auto raw) { return FinalizationEpoch(raw); },
				str,
				parsedValue);
	}

	bool TryParseValue(const std::string& str, MosaicId& parsedValue) {
		return TryParseCustomUnsignedIntHexValue<MosaicId::ValueType>([](auto raw) { return MosaicId(raw); }, str, parsedValue);
	}

	bool TryParseValue(const std::string& str, TimeSpan& parsedValue) {
		if (str.empty())
			return false;

		auto tryParse = [&str, &parsedValue](const auto& factory, uint8_t postfixSize) {
			return TryParseCustomUnsignedIntDecimalValue<uint64_t>(factory, str.substr(0, str.size() - postfixSize), parsedValue);
		};

		switch (str.back()) {
		case 's':
			if (str.size() > 2 && 'm' == str[str.size() - 2])
				return tryParse(TimeSpan::FromMilliseconds, 2);

			return tryParse(TimeSpan::FromSeconds, 1);

		case 'm':
			return tryParse(TimeSpan::FromMinutes, 1);

		case 'h':
			return tryParse(TimeSpan::FromHours, 1);
		}

		return false;
	}

	bool TryParseValue(const std::string& str, BlockSpan& parsedValue) {
		if (str.size() < 2)
			return false;

		auto tryParse = [&str, &parsedValue](const auto& factory) {
			return TryParseCustomUnsignedIntDecimalValue<uint64_t>(factory, str.substr(0, str.size() - 1), parsedValue);
		};

		switch (str[str.size() - 1]) {
		case 'm':
			return tryParse(BlockSpan::FromMinutes);

		case 'h':
			return tryParse(BlockSpan::FromHours);

		case 'd':
			return tryParse(BlockSpan::FromDays);
		}

		return false;
	}

	bool TryParseValue(const std::string& str, FileSize& parsedValue) {
		if (str.size() < 2 || 'B' != str.back())
			return false;

		auto tryParse = [&str, &parsedValue](const auto& factory, uint8_t postfixSize) {
			return TryParseCustomUnsignedIntDecimalValue<uint64_t>(factory, str.substr(0, str.size() - postfixSize), parsedValue);
		};

		switch (str[str.size() - 2]) {
		case 'K':
			return tryParse(FileSize::FromKilobytes, 2);

		case 'M':
			return tryParse(FileSize::FromMegabytes, 2);

		default:
			return tryParse(FileSize::FromBytes, 1);
		}
	}

	// endregion

	// region byte array

	namespace {
		template<typename TByteArray>
		bool TryParseByteArray(const std::string& str, TByteArray& parsedValue) {
			TByteArray array;
			if (!TryParseHexStringIntoContainer(str.data(), str.size(), array))
				return false;

			parsedValue = array;
			return true;
		}
	}

	bool TryParseValue(const std::string& str, Key& parsedValue) {
		return TryParseByteArray(str, parsedValue);
	}

	bool TryParseValue(const std::string& str, Hash256& parsedValue) {
		return TryParseByteArray(str, parsedValue);
	}

	bool TryParseValue(const std::string& str, GenerationHashSeed& parsedValue) {
		return TryParseByteArray(str, parsedValue);
	}

	// endregion

	// region string

	bool TryParseValue(const std::string& str, std::string& parsedValue) {
		parsedValue = str;
		return true;
	}

	// endregion

	// region set

	namespace {
		std::string Trim(const std::string& str) {
			constexpr auto Whitespace = " \t";
			auto startIndex = str.find_first_not_of(Whitespace);
			if (std::string::npos == startIndex)
				return std::string();

			auto endIndex = str.find_last_not_of(Whitespace);
			return str.substr(startIndex, endIndex - startIndex + 1);
		}
	}

	bool TryParseValue(const std::string& str, std::unordered_set<std::string>& parsedValue) {
		if (str.empty()) {
			parsedValue.clear();
			return true;
		}

		size_t searchIndex = 0;
		std::unordered_set<std::string> values;
		while (true) {
			auto separatorIndex = str.find(',', searchIndex);
			auto item = std::string::npos == separatorIndex
					? str.substr(searchIndex)
					: str.substr(searchIndex, separatorIndex - searchIndex);

			// don't allow empty values or duplicates
			item = Trim(item);
			if (item.empty() || !values.emplace(item).second)
				return false;

			if (std::string::npos == separatorIndex)
				break;

			searchIndex = separatorIndex + 1;
		}

		parsedValue = std::move(values);
		return true;
	}

	bool TryParseValue(const std::string& str, std::unordered_set<Height, BaseValueHasher<Height>>& parsedValue) {
		std::unordered_set<std::string> strParts;
		if (!TryParseValue(str, strParts))
			return false;

		std::unordered_set<Height, BaseValueHasher<Height>> heights;
		for (const auto& strPart : strParts) {
			Height height;
			if (!TryParseValue(strPart, height))
				return false;

			heights.emplace(height);
		}

		parsedValue = std::move(heights);
		return true;
	}

	// endregion
}}
