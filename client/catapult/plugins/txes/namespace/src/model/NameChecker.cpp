#include "NameChecker.h"
#include <algorithm>

namespace catapult { namespace model {

	namespace {
		constexpr bool IsAlphaNumeric(uint8_t ch) {
			return (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9');
		}

		constexpr bool IsValidChar(uint8_t ch) {
			return IsAlphaNumeric(ch) || '-' == ch || '_' == ch;
		}
	}

	bool IsValidName(const uint8_t* pName, size_t nameSize) {
		if (0 == nameSize)
			return false;

		return IsAlphaNumeric(pName[0]) && std::all_of(pName + 1, pName + nameSize, IsValidChar);
	}
}}
