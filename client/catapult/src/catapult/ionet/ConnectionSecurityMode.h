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

#pragma once
#include "catapult/utils/BitwiseEnum.h"
#include <iosfwd>

namespace catapult { namespace ionet {

#define CONNECTION_SECURITY_MODE_LIST \
	/* Connection allows regular packets. */ \
	ENUM_VALUE(None, 1) \
	\
	/* Connection only allows signed packets. */ \
	ENUM_VALUE(Signed, 2)

#define ENUM_VALUE(LABEL, VALUE) LABEL = VALUE,
	/// Possible connection security modes.
	enum class ConnectionSecurityMode : uint8_t {
		CONNECTION_SECURITY_MODE_LIST
	};
#undef ENUM_VALUE

	MAKE_BITWISE_ENUM(ConnectionSecurityMode)

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, ConnectionSecurityMode value);

	/// Tries to parse \a str into connection security \a modes.
	bool TryParseValue(const std::string& str, ConnectionSecurityMode& modes);
}}
