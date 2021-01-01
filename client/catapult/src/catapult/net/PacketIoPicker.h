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
#include "catapult/ionet/NodePacketIoPair.h"
#include <memory>

namespace catapult { namespace utils { class TimeSpan; } }

namespace catapult { namespace net {

	/// Interface for picking packet io pairs.
	class PLUGIN_API_DEPENDENCY PacketIoPicker {
	public:
		virtual ~PacketIoPicker() = default;

	public:
		/// Retrieves a packet io pair around an active connection or an empty pair if no connections are available.
		/// After \a ioDuration elapses, the connection will timeout.
		virtual ionet::NodePacketIoPair pickOne(const utils::TimeSpan& ioDuration) = 0;
	};

	/// Retrieves io interfaces to at most \a numRequested connections from \a picker.
	/// After \a ioDuration elapses, the connections will timeout.
	std::vector<ionet::NodePacketIoPair> PickMultiple(PacketIoPicker& picker, size_t numRequested, const utils::TimeSpan& ioDuration);
}}
