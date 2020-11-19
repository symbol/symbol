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
#include "PacketIoPicker.h"
#include <vector>

namespace catapult { namespace net {

	/// Collection of packet io pickers.
	class PacketIoPickerContainer {
	public:
		/// Inserts \a picker, which is compatible with the specified \a roles, into this container.
		void insert(PacketIoPicker& picker, ionet::NodeRoles roles);

		/// Retrieves a packet io pair around an active connection from each picker with compatible \a roles.
		/// After \a ioDuration elapses, the connections will timeout.
		std::vector<ionet::NodePacketIoPair> pickMatching(const utils::TimeSpan& ioDuration, ionet::NodeRoles roles) const;

	private:
		std::vector<std::pair<ionet::NodeRoles, PacketIoPicker*>> m_pickers;
	};
}}
