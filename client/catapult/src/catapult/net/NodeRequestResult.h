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

namespace catapult { namespace net {

#define NODE_REQUEST_RESULT_LIST \
	/* Connection to the remote node failed. */ \
	ENUM_VALUE(Failure_Connection) \
	\
	/* Interaction with the remote node failed. */ \
	ENUM_VALUE(Failure_Interaction) \
	\
	/* Remote response is incompatible with the local node. */ \
	ENUM_VALUE(Failure_Incompatible) \
	\
	/* Interaction with the remote node timed out. */ \
	ENUM_VALUE(Failure_Timeout) \
	\
	/* Ping operation succeeded. */ \
	ENUM_VALUE(Success)

#define ENUM_VALUE(LABEL) LABEL,
	/// Enumeration of possible results of a ping operation.
	enum class NodeRequestResult {
		NODE_REQUEST_RESULT_LIST
	};
#undef ENUM_VALUE

	/// Insertion operator for outputting \a value to \a out.
	std::ostream& operator<<(std::ostream& out, NodeRequestResult value);
}}
