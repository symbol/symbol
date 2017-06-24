#include "catapult/crypto/KeyUtils.h"
#include "catapult/crypto/PrivateKey.h"
#include "tests/TestHarness.h"
#include <array>

namespace catapult { namespace crypto {

	namespace {
		template<typename T>
		std::string FormatKeyAsString(const T& key) {
			std::ostringstream out;
			out << FormatKey(key);
			return out.str();
		}
	}

	TEST(KeyUtilsTests, CanOutputPublicKey) {
		// Arrange:
		auto key = ParseKey("031729d10db52ecf0ad3684558db31895ddfa5cd7f4143af6e822e114e16e31c");

		// Act:
		std::string actual = FormatKeyAsString(key);

		// Assert:
		EXPECT_EQ("031729D10DB52ECF0AD3684558DB31895DDFA5CD7F4143AF6E822E114E16E31C", actual);
	}

	TEST(KeyUtilsTests, CanOutputPrivateKey) {
		// Arrange:
		auto key = PrivateKey::FromString("031729d10db52ecf0ad3684558db31895ddfa5cd7f4143af6e822e114e16e31c");

		// Act:
		std::string actual = FormatKeyAsString(key);

		// Assert:
		EXPECT_EQ("031729D10DB52ECF0AD3684558DB31895DDFA5CD7F4143AF6E822E114E16E31C", actual);
	}

	TEST(KeyUtilsTests, CanParseValidKeyString) {
		// Act:
		auto key = ParseKey("031729d10db52ecf0ad3684558db31895ddfa5cd7f4143af6e822e114e16e31c");

		// Assert:
		Key expected = {{
			0x03, 0x17, 0x29, 0xD1, 0x0D, 0xB5, 0x2E, 0xCF, 0x0A, 0xD3, 0x68, 0x45, 0x58, 0xDB, 0x31, 0x89,
			0x5D, 0xDF, 0xA5, 0xCD, 0x7F, 0x41, 0x43, 0xAF, 0x6E, 0x82, 0x2E, 0x11, 0x4E, 0x16, 0xE3, 0x1C
		}};
		EXPECT_EQ(expected, key);
	}

	TEST(KeyUtilsTests, CannotParseInvalidKeyString) {
		// Act:
		EXPECT_THROW(ParseKey("031729d10db52ect0ad3684558db31895ddfa5cd7f4143af6e822e114e16e31c"), catapult_invalid_argument);
	}
}}
