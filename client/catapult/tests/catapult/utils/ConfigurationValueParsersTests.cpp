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

#include "catapult/utils/ConfigurationValueParsers.h"
#include "catapult/utils/BlockSpan.h"
#include "catapult/utils/FileSize.h"
#include "catapult/utils/TimeSpan.h"
#include "tests/test/nodeps/ConfigurationTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace utils {

#define TEST_CLASS ConfigurationValueParsersTests

	namespace {
		template<typename T>
		bool TryParseValueT(const std::string& input, T& parsedValue) {
			return TryParseValue(input, parsedValue);
		}

		template<typename T>
		void AssertSuccessfulParse(const std::string& input, const T& expectedParsedValue) {
			test::AssertParse(input, expectedParsedValue, TryParseValueT<T>);
		}

		template<typename T>
		void AssertFailedParse(const std::string& input, const T& initialValue) {
			test::AssertFailedParse(input, initialValue, TryParseValueT<T>);
		}
	}

	// region discrete enums

	namespace {
		template<typename T>
		void AssertEnumParseFailure(const std::string& seed, const T& initialValue) {
			test::AssertEnumParseFailure(seed, initialValue, TryParseValueT<T>);
		}

		const std::array<std::pair<const char*, int>, 4> String_To_Square_Mapping{{
			{ std::make_pair("one", 1) },
			{ std::make_pair("two", 4) },
			{ std::make_pair("three", 9) },
			{ std::make_pair("four", 16) },
		}};
	}

	TEST(TEST_CLASS, CanParseValidEnumValue) {
		// Arrange:
		auto assertSuccessfulParse = [](const auto& input, const auto& expectedParsedValue) {
			test::AssertParse(input, expectedParsedValue, [](const auto& str, auto& parsedValue) {
				return TryParseEnumValue(String_To_Square_Mapping, str, parsedValue);
			});
		};

		// Assert:
		assertSuccessfulParse("one", 1);
		assertSuccessfulParse("two", 4);
		assertSuccessfulParse("three", 9);
		assertSuccessfulParse("four", 16);
	}

	TEST(TEST_CLASS, CannotParseInvalidEnumValue) {
		// Assert:
		test::AssertEnumParseFailure("two", 7, [](const auto& str, auto& parsedValue) {
			return TryParseEnumValue(String_To_Square_Mapping, str, parsedValue);
		});
	}

	// endregion

	// region bitwise enums

	TEST(TEST_CLASS, CanParseEmptyStringAsBitwiseEnumValue) {
		// Act + Assert:
		test::AssertParse("", 0, [](const auto& str, auto& parsedValue) {
			return TryParseBitwiseEnumValue(String_To_Square_Mapping, str, parsedValue);
		});
	}

	TEST(TEST_CLASS, CanParseSingleValueAsBitwiseEnumValue) {
		// Act + Assert:
		test::AssertParse("three", 9, [](const auto& str, auto& parsedValue) {
			return TryParseBitwiseEnumValue(String_To_Square_Mapping, str, parsedValue);
		});
	}

	TEST(TEST_CLASS, CanParseMultipleValuesAsBitwiseEnumValue) {
		// Act + Assert:
		test::AssertParse("two,four", 20, [](const auto& str, auto& parsedValue) {
			return TryParseBitwiseEnumValue(String_To_Square_Mapping, str, parsedValue);
		});
	}

	TEST(TEST_CLASS, CannotParseMalformedSetAsBitwiseEnumValue) {
		// Act + Assert:
		test::AssertFailedParse("two,,four", 0, [](const auto& str, auto& parsedValue) {
			return TryParseBitwiseEnumValue(String_To_Square_Mapping, str, parsedValue);
		});
	}

	TEST(TEST_CLASS, CannotParseSetWithUnknownValueAsBitwiseEnumValue) {
		// Act + Assert:
		test::AssertFailedParse("two,five,four", 0, [](const auto& str, auto& parsedValue) {
			return TryParseBitwiseEnumValue(String_To_Square_Mapping, str, parsedValue);
		});
	}

	// endregion

	// region log related enums

	TEST(TEST_CLASS, CanParseValidLogLevel) {
		// Assert:
		using T = LogLevel;
		AssertSuccessfulParse("Trace", T::Trace);
		AssertSuccessfulParse("Debug", T::Debug);
		AssertSuccessfulParse("Info", T::Info);
		AssertSuccessfulParse("Warning", T::Warning);
		AssertSuccessfulParse("Error", T::Error);
		AssertSuccessfulParse("Fatal", T::Fatal);
		AssertSuccessfulParse("Min", T::Min);
		AssertSuccessfulParse("Max", T::Max);
	}

	TEST(TEST_CLASS, CannotParseInvalidLogLevel) {
		// Assert:
		AssertEnumParseFailure("Warning", LogLevel::Info);
	}

	TEST(TEST_CLASS, CanParseValidLogSinkType) {
		// Assert:
		using T = LogSinkType;
		AssertSuccessfulParse("Sync", T::Sync);
		AssertSuccessfulParse("Async", T::Async);
	}

	TEST(TEST_CLASS, CannotParseInvalidLogSinkType) {
		// Assert:
		AssertEnumParseFailure("Sync", LogSinkType::Async);
	}

	TEST(TEST_CLASS, CanParseValidLogColorMode) {
		// Assert:
		using T = LogColorMode;
		AssertSuccessfulParse("Ansi", T::Ansi);
		AssertSuccessfulParse("AnsiBold", T::AnsiBold);
		AssertSuccessfulParse("None", T::None);
	}

	TEST(TEST_CLASS, CannotParseInvalidLogColorMode) {
		// Assert:
		AssertEnumParseFailure("Ansi", LogColorMode::None);
	}

	// endregion

	// region bool

	TEST(TEST_CLASS, CanParseValidBoolean) {
		// Assert:
		AssertSuccessfulParse("true", true);
		AssertSuccessfulParse("false", false);
	}

	TEST(TEST_CLASS, CannotParseInvalidBoolean) {
		// Assert:
		AssertEnumParseFailure("false", true);
	}

	// endregion

	// region int

	namespace {
		template<typename TNumeric, typename TFactory>
		void AssertUnsignedIntParseSuccess(TNumeric expectedMaxValue, const std::string& postfix, TFactory factory) {
			using NumericLimits = std::numeric_limits<TNumeric>;
			AssertSuccessfulParse(std::to_string(NumericLimits::min()) + postfix, factory(static_cast<TNumeric>(0))); // min
			AssertSuccessfulParse("1" + postfix, factory(1)); // other values
			AssertSuccessfulParse("1234" + postfix, factory(1234));
			AssertSuccessfulParse("8692" + postfix, factory(8692));
			AssertSuccessfulParse("8'692" + postfix, factory(8692)); // with separators
			AssertSuccessfulParse("8'6'9'2" + postfix, factory(8692));
			AssertSuccessfulParse(std::to_string(NumericLimits::max()) + postfix, factory(expectedMaxValue)); // max
		}

		template<typename T, typename TNumeric>
		void AssertUnsignedIntParseSuccess(TNumeric expectedMaxValue) {
			AssertUnsignedIntParseSuccess<TNumeric>(expectedMaxValue, "", [](auto raw) { return T(static_cast<TNumeric>(raw)); });
		}

		template<typename T>
		void AssertUnsignedIntParseSuccess(T expectedMaxValue) {
			AssertUnsignedIntParseSuccess<T, T>(expectedMaxValue);
		}

		template<typename T, typename TNumeric>
		void AssertUnsignedIntParseFailure(const T& initialValue, const std::string& postfix) {
			using NumericLimits = std::numeric_limits<TNumeric>;
			AssertFailedParse("", initialValue); // empty
			AssertFailedParse(postfix, initialValue); // postfix only
			AssertFailedParse("'1234" + postfix, initialValue); // leading separator
			AssertFailedParse("1234'" + postfix, initialValue); // trailing separator
			AssertFailedParse("1''234" + postfix, initialValue); // consecutive separators
			AssertFailedParse(" 1234" + postfix, initialValue); // leading space
			AssertFailedParse("1234" + postfix + " ", initialValue); // trailing space
			AssertFailedParse("-1" + postfix, initialValue); // too small

			// too large
			auto maxString = std::to_string(NumericLimits::max());
			AssertFailedParse(maxString + "0" + postfix, initialValue); // 10x too large
			++maxString.back();
			AssertFailedParse(maxString + postfix, initialValue); // 1 too large

			AssertFailedParse("10.25" + postfix, initialValue); // non integral
			AssertFailedParse("10A25" + postfix, initialValue); // non decimal digit
			AssertFailedParse("abc" + postfix, initialValue); // non numeric
		}

		template<typename T, typename TNumeric>
		void AssertUnsignedIntParseFailure() {
			AssertUnsignedIntParseFailure<T, TNumeric>(T(177), "");
		}

		template<typename T>
		void AssertUnsignedIntParseFailure() {
			AssertUnsignedIntParseFailure<T, T>();
		}
	}

	TEST(TEST_CLASS, CanParseValidUInt8) {
		// Assert:
		AssertUnsignedIntParseSuccess(static_cast<uint8_t>(0xFF));
	}

	TEST(TEST_CLASS, CannotParseInvalidUInt8) {
		// Assert:
		AssertUnsignedIntParseFailure<uint8_t>();
	}

	TEST(TEST_CLASS, CanParseValidUInt16) {
		// Assert:
		AssertUnsignedIntParseSuccess(static_cast<uint16_t>(0xFFFF));
	}

	TEST(TEST_CLASS, CannotParseInvalidUInt16) {
		// Assert:
		AssertUnsignedIntParseFailure<uint16_t>();
	}

	TEST(TEST_CLASS, CanParseValidUInt32) {
		// Assert:
		AssertUnsignedIntParseSuccess(static_cast<uint32_t>(0xFFFF'FFFF));
	}

	TEST(TEST_CLASS, CannotParseInvalidUInt32) {
		// Assert:
		AssertUnsignedIntParseFailure<uint32_t>();
	}

	TEST(TEST_CLASS, CanParseValidUInt64) {
		// Assert:
		AssertUnsignedIntParseSuccess(static_cast<uint64_t>(0xFFFF'FFFF'FFFF'FFFF));
	}

	TEST(TEST_CLASS, CannotParseInvalidUInt64) {
		// Assert:
		AssertUnsignedIntParseFailure<uint64_t>();
	}

	// endregion

	// region custom types

	TEST(TEST_CLASS, CanParseValidAmount) {
		// Assert:
		AssertUnsignedIntParseSuccess<Amount, Amount::ValueType>(0xFFFF'FFFF'FFFF'FFFF);
	}

	TEST(TEST_CLASS, CannotParseInvalidAmount) {
		// Assert:
		AssertUnsignedIntParseFailure<Amount, Amount::ValueType>();
	}

	TEST(TEST_CLASS, CanParseValidTimeSpan) {
		// Assert:
		auto maxValue = 0xFFFF'FFFF'FFFF'FFFF;
		AssertUnsignedIntParseSuccess<uint64_t>(maxValue, "ms", TimeSpan::FromMilliseconds);
		AssertUnsignedIntParseSuccess<uint64_t>(maxValue, "s", TimeSpan::FromSeconds);
		AssertUnsignedIntParseSuccess<uint64_t>(maxValue, "m", TimeSpan::FromMinutes);
		AssertUnsignedIntParseSuccess<uint64_t>(maxValue, "h", TimeSpan::FromHours);
	}

	TEST(TEST_CLASS, CannotParseInvalidTimeSpan) {
		// Assert:
		auto initialValue = TimeSpan::FromMinutes(8888);
		AssertUnsignedIntParseFailure<TimeSpan, uint64_t>(initialValue, "m");

		AssertFailedParse("1234", initialValue); // no specifier
		AssertFailedParse("12s34", initialValue); // non-terminating specifier
		AssertFailedParse("1234S", initialValue); // wrong case specifier
		AssertFailedParse("1234d", initialValue); // unknown specifier
	}

	TEST(TEST_CLASS, CanParseValidBlockSpan) {
		// Assert:
		auto maxValue = 0xFFFF'FFFF'FFFF'FFFF;
		AssertUnsignedIntParseSuccess<uint64_t>(maxValue, "h", BlockSpan::FromHours);
		AssertUnsignedIntParseSuccess<uint64_t>(maxValue, "d", BlockSpan::FromDays);
	}

	TEST(TEST_CLASS, CannotParseInvalidBlockSpan) {
		// Assert:
		auto initialValue = BlockSpan::FromHours(8888);
		AssertUnsignedIntParseFailure<BlockSpan, uint64_t>(initialValue, "h");

		AssertFailedParse("1234", initialValue); // no specifier
		AssertFailedParse("12h34", initialValue); // non-terminating specifier
		AssertFailedParse("1234H", initialValue); // wrong case specifier
		AssertFailedParse("1234s", initialValue); // unknown specifier
	}

	TEST(TEST_CLASS, CanParseValidFileSize) {
		// Assert:
		auto maxValue = 0xFFFF'FFFF'FFFF'FFFF;
		AssertUnsignedIntParseSuccess<uint64_t>(maxValue, "B", FileSize::FromBytes);
		AssertUnsignedIntParseSuccess<uint64_t>(maxValue, "KB", FileSize::FromKilobytes);
		AssertUnsignedIntParseSuccess<uint64_t>(maxValue, "MB", FileSize::FromMegabytes);
	}

	TEST(TEST_CLASS, CannotParseInvalidFileSize) {
		// Assert:
		auto initialValue = FileSize::FromKilobytes(8888);
		AssertUnsignedIntParseFailure<FileSize, uint64_t>(initialValue, "KB");

		AssertFailedParse("1234", initialValue); // no specifier
		AssertFailedParse("12KB34", initialValue); // non-terminating specifier
		AssertFailedParse("1234Kb", initialValue); // wrong case specifier
		AssertFailedParse("1234GB", initialValue); // unknown specifier
	}

	// endregion

	// region key

	TEST(TEST_CLASS, CanParseValidKey) {
		// Assert:
		AssertSuccessfulParse("031729D10DB52ECF0AD3684558DB31895DDFA5CD7F4143AF6E822E114E16E31C", Key{{
			0x03, 0x17, 0x29, 0xD1, 0x0D, 0xB5, 0x2E, 0xCF, 0x0A, 0xD3, 0x68, 0x45, 0x58, 0xDB, 0x31, 0x89,
			0x5D, 0xDF, 0xA5, 0xCD, 0x7F, 0x41, 0x43, 0xAF, 0x6E, 0x82, 0x2E, 0x11, 0x4E, 0x16, 0xE3, 0x1C
		}});
		AssertSuccessfulParse("AB1729D10DB52ECF0AD3684558DB31895DDFA5CD7F4143AF6E822E114E16E300", Key{{
			0xAB, 0x17, 0x29, 0xD1, 0x0D, 0xB5, 0x2E, 0xCF, 0x0A, 0xD3, 0x68, 0x45, 0x58, 0xDB, 0x31, 0x89,
			0x5D, 0xDF, 0xA5, 0xCD, 0x7F, 0x41, 0x43, 0xAF, 0x6E, 0x82, 0x2E, 0x11, 0x4E, 0x16, 0xE3, 0x00
		}});
	}

	TEST(TEST_CLASS, CannotParseInvalidKey) {
		// Arrange:
		Key initialValue{ { 0x25 } };

		// Assert
		AssertFailedParse("031729D10DB52ECF0AD3684558DB3189@DDFA5CD7F4143AF6E822E114E16E31C", initialValue); // invalid char
		AssertFailedParse("31729D10DB52ECF0AD3684558DB31895DDFA5CD7F4143AF6E822E114E16E31C", initialValue); // too short (odd)
		AssertFailedParse("1729D10DB52ECF0AD3684558DB31895DDFA5CD7F4143AF6E822E114E16E31C", initialValue); // too short (even)
		AssertFailedParse("031729D10DB52ECF0AD3684558DB31895DDFA5CD7F4143AF6E822E114E16E31CA", initialValue); // too long (odd)
		AssertFailedParse("031729D10DB52ECF0AD3684558DB31895DDFA5CD7F4143AF6E822E114E16E31CAB", initialValue); // too long (even)
	}

	// endregion

	// region string

	TEST(TEST_CLASS, CanParseString) {
		// Assert:
		AssertSuccessfulParse("Foo BAR", std::string("Foo BAR"));
		AssertSuccessfulParse("Ac$D*a98p124!", std::string("Ac$D*a98p124!"));
	}

	// endregion

	// region set

	TEST(TEST_CLASS, CanParseValidUnorderedSetOfString) {
		// Arrange:
		using Container = std::unordered_set<std::string>;

		// Assert:
		AssertSuccessfulParse("", Container()); // no values
		AssertSuccessfulParse("alpha", Container{ "alpha" });
		AssertSuccessfulParse("alpha,bEta,gammA", Container{ "alpha", "bEta", "gammA" });
		AssertSuccessfulParse("\talpha\t,  bEta  "", gammA,zeta ", Container{ "alpha", "bEta", "gammA", "zeta" });
		AssertSuccessfulParse("Foo BAR,Ac$D*a98p124!", Container{ "Foo BAR", "Ac$D*a98p124!" });
	}

	TEST(TEST_CLASS, CannotParseInvalidUnorderedSetOfString) {
		// Arrange:
		std::unordered_set<std::string> initialValue{ "default", "values" };

		// Assert
		AssertFailedParse(",", initialValue); // no values
		AssertFailedParse("alpha,,gammA", initialValue); // empty value (middle)
		AssertFailedParse("alpha,gammA,", initialValue); // empty value (last)
		AssertFailedParse(",alpha,gammA", initialValue); // empty value (first)
		AssertFailedParse("alpha, \t \t,gammA", initialValue); // whitespace value
		AssertFailedParse("alpha,beta,alpha", initialValue); // duplicate values
	}

	// endregion
}}
