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
#include "Packet.h"
#include "PacketPayload.h"
#include "SocketOperationCode.h"
#include "catapult/functions.h"
#include <memory>

namespace catapult { namespace ionet {

	/// Interface for reading and writing packets.
	class PLUGIN_API_DEPENDENCY PacketIo {
	public:
		using ReadCallback = consumer<SocketOperationCode, const Packet*>;
		using WriteCallback = consumer<SocketOperationCode>;

	public:
		virtual ~PacketIo() = default;

	public:
		/// Writes \a payload and calls \a callback on completion.
		virtual void write(const PacketPayload& payload, const WriteCallback& callback) = 0;

		/// Reads and consumes the next packet and calls \a callback on completion.
		/// On success, the read packet is passed to \a callback.
		virtual void read(const ReadCallback& callback) = 0;
	};
}}
