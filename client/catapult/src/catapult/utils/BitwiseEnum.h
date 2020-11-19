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
#include "Casting.h"

/// Defines bitwise helpers for \a ENUM_TYPE.
#define MAKE_BITWISE_ENUM(ENUM_TYPE) \
	/* Returns the result of bitwise-ORing \a lhs and \a rhs. */ \
	constexpr ENUM_TYPE operator|(ENUM_TYPE lhs, ENUM_TYPE rhs) { \
		return static_cast<ENUM_TYPE>(utils::to_underlying_type(lhs) | utils::to_underlying_type(rhs)); \
	} \
	\
	/* Bitwise-ORs \a lhs and \a rhs and assigns the result to \a lhs. */ \
	constexpr ENUM_TYPE& operator|=(ENUM_TYPE& lhs, ENUM_TYPE rhs) { \
		return lhs = lhs | rhs; \
	} \
	\
	/* Returns \c true if all bits in \a testedFlag are also set in \a value. */ \
	constexpr bool HasFlag(ENUM_TYPE testedFlag, ENUM_TYPE value) { \
		return utils::to_underlying_type(testedFlag) == (utils::to_underlying_type(testedFlag) & utils::to_underlying_type(value)); \
	} \
	/* Returns true if \a value has exactly one bit set. */ \
	constexpr bool HasSingleFlag(ENUM_TYPE value) { \
		return utils::to_underlying_type(value) && !(utils::to_underlying_type(value) & (utils::to_underlying_type(value) - 1)); \
	}
