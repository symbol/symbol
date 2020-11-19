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
#include <iosfwd>

namespace catapult { namespace ionet {

#define SOCKET_OPERATION_CODE_LIST \
	/* Socket operation succeeded. */ \
	ENUM_VALUE(Success) \
	\
	/* Socket was closed. */ \
	ENUM_VALUE(Closed) \
	\
	/* Socket produced a read error. */ \
	ENUM_VALUE(Read_Error) \
	\
	/* Socket produced a write error. */ \
	ENUM_VALUE(Write_Error) \
	\
	/* Socket produced malformed data. */ \
	ENUM_VALUE(Malformed_Data) \
	\
	/* Socket produced a security error. */ \
	ENUM_VALUE(Security_Error) \
	\
	/* Socket operation completed due to insufficient data. */ \
	ENUM_VALUE(Insufficient_Data)

#define ENUM_VALUE(LABEL) LABEL,
	/// Enumeration of socket operation results.
	enum class SocketOperationCode {
		SOCKET_OPERATION_CODE_LIST
	};
#undef ENUM_VALUE

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, SocketOperationCode value);
}}
