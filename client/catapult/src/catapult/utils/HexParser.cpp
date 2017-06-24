#include "HexParser.h"
#include <string>

namespace catapult { namespace utils {

	namespace {
		bool TryParseNibble(const char ch, int& nibble) {
			if (ch >= 'A' && ch <= 'F')
				nibble = ch - 'A' + 10;
			else if (ch >= 'a' && ch <= 'f')
				nibble = ch - 'a' + 10;
			else if (ch >= '0' && ch <= '9')
				nibble = ch - '0';
			else
				return false;

			return true;
		}
	}

	uint8_t ParseByte(char ch1, char ch2) {
		uint8_t by;
		if (!TryParseByte(ch1, ch2, by)) {
			auto byteString = std::string{ ch1, ch2 };
			CATAPULT_THROW_INVALID_ARGUMENT_1("unknown hex character in string", byteString);
		}

		return by;
	}

	bool TryParseByte(char ch1, char ch2, uint8_t& by) {
		int nibble1, nibble2;
		if (!TryParseNibble(ch1, nibble1) || !TryParseNibble(ch2, nibble2))
			return false;

		by = static_cast<uint8_t>(nibble1 << 4 | nibble2);
		return true;
	}
}}
