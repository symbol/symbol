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

#ifdef DEFINE_ENUM
#define ENUM_TYPE DEFINE_ENUM
#endif

#if !defined(ENUM_LIST) || !defined(ENUM_TYPE)
#error "In order to use MacroBasedEnum.h ENUM_LIST and DEFINE_ENUM must be defined"
#endif

#ifdef DEFINE_ENUM
#define CONCAT_SYMBOLS(LEFT, RIGHT) LEFT##RIGHT
#define QUOTE(NAME) #NAME
#define STR(MACRO) QUOTE(MACRO)

	namespace {
		const char* CONCAT_SYMBOLS(ENUM_TYPE, ToString)(ENUM_TYPE value) {
			switch (value) {
#ifdef EXPLICIT_VALUE_ENUM
#define ENUM_VALUE(LABEL, VALUE) case ENUM_TYPE::LABEL: return #LABEL;
#else
#define ENUM_VALUE(LABEL) case ENUM_TYPE::LABEL: return #LABEL;
#endif

			ENUM_LIST

#undef ENUM_VALUE
			}

			return nullptr;
		}
	}

	std::ostream& operator<<(std::ostream& out, ENUM_TYPE value) {
		auto pLabel = CONCAT_SYMBOLS(ENUM_TYPE, ToString)(value);
		if (pLabel)
			out << pLabel;
		else
			out << STR(ENUM_TYPE) << "(0x" << utils::HexFormat(utils::to_underlying_type(value)) << ")";
		return out;
	}

#undef STR
#undef QUOTE
#undef CONCAT_SYMBOLS
#endif

#undef ENUM_TYPE
