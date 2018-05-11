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
#include "catapult/ionet/IoTypes.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/types.h"
#include <memory>

namespace boost { namespace asio { class io_service; } }

namespace catapult { namespace net { class PacketWriters; } }

namespace catapult { namespace test {

	/// Creates a connection to localhost on \a port configured with server public key \a serverPublicKey using \a service.
	std::shared_ptr<ionet::PacketSocket> ConnectToLocalHost(
			boost::asio::io_service& service,
			unsigned short port,
			const Key& serverPublicKey);

	/// Creates a connection to localhost configured with server public key \a serverPublicKey
	/// using \a packetWriters.
	void ConnectToLocalHost(net::PacketWriters& packetWriters, const Key& serverPublicKey);

	/// Starts an async read on \a io that fills \a buffer using \a service.
	void AsyncReadIntoBuffer(boost::asio::io_service& service, ionet::PacketSocket& io, ionet::ByteBuffer& buffer);
}}
