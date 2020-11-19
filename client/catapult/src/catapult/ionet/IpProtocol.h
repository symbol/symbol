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
#include "NodeRoles.h"

namespace catapult { namespace ionet {

	/// IP protocols.
	enum class IpProtocol : uint8_t {
		/// No protocols.
		None = 0x00,

		/// IPv4.
		IPv4 = 0x01,

		/// IPv6.
		IPv6 = 0x02,

		/// All protocols.
		All = 0xFF
	};

	MAKE_BITWISE_ENUM(IpProtocol)

	/// Map \a roles to IP protocols.
	IpProtocol MapNodeRolesToIpProtocols(NodeRoles roles);

	/// Returns \c true if \a roles supports any protocol in \a protocols.
	bool HasAnyProtocol(IpProtocol protocols, NodeRoles roles);
}}
