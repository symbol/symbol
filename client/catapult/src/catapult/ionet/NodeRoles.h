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
#include <string>

namespace catapult { namespace ionet {

	/// Node's role.
	enum class NodeRoles : uint32_t {
		/// No roles.
		None,

		/// Peer node.
		Peer = 0x01,

		/// Api node.
		Api = 0x02,

		/// Voting node.
		Voting = 0x04
	};

	MAKE_BITWISE_ENUM(NodeRoles)

	/// Tries to parse \a str into node \a roles.
	bool TryParseValue(const std::string& str, NodeRoles& roles);
}}
