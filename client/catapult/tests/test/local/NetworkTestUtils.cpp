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

#include "NetworkTestUtils.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/net/PacketWriters.h"
#include "catapult/thread/TimedCallback.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/net/SocketTestUtils.h"
#include "tests/test/nodeps/KeyTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	std::shared_ptr<ionet::PacketSocket> ConnectToLocalHost(boost::asio::io_context& ioContext, unsigned short port) {
		// Act: connect to the server
		std::atomic_bool isConnected(false);
		auto options = CreatePacketSocketOptions(GenerateRandomByteArray<Key>());
		auto endpoint = CreateLocalHostNodeEndpoint(port);
		std::shared_ptr<ionet::PacketSocket> pIo;
		ionet::Connect(ioContext, options, endpoint, [&isConnected, &pIo](auto connectCode, const auto& connectedSocketInfo) {
			CATAPULT_LOG(debug) << "node is connected with code " << connectCode;
			pIo = connectedSocketInfo.socket();
			if (!pIo)
				return;

			isConnected = true;
		});
		WAIT_FOR(isConnected);
		return pIo;
	}

	void ConnectToLocalHost(net::PacketWriters& packetWriters, const Key& serverPublicKey) {
		// Act: connect to the server
		net::PeerConnectCode connectCode;
		std::atomic<size_t> numConnects(0);
		packetWriters.connect(CreateLocalHostNode(serverPublicKey), [&](const auto& connectResult) {
			connectCode = connectResult.Code;
			++numConnects;
		});
		WAIT_FOR_ONE(numConnects);

		// Assert: a single connection was accepted
		EXPECT_EQ(net::PeerConnectCode::Accepted, connectCode);
		EXPECT_EQ(1u, numConnects);
	}

	void AsyncReadIntoBuffer(boost::asio::io_context& ioContext, ionet::PacketSocket& io, ionet::ByteBuffer& buffer) {
		// set up a timer that will close the socket if it takes too long to respond to read
		// a TimedCallback can't be passed directly to io.read because the TimedCallback would be called by the socket handler and
		// then dispatch to a separate thread (to execute in context of the TimedCallback strand); this switching would allow the
		// socket read handler to potentially complete (and invalidate the read packet pointer) before the user callback is called
		auto pTimedCallback = thread::MakeTimedCallback(ioContext, consumer<bool>([](auto) {}), false);
		pTimedCallback->setTimeout(utils::TimeSpan::FromSeconds(detail::Default_Wait_Timeout));
		pTimedCallback->setTimeoutHandler([&io]() {
			CATAPULT_LOG(warning) << "closing socket due to timeout";
			io.close();
		});

		io.read([&buffer, pTimedCallback](auto code, const auto* pPacket) {
			// mark the operation completed and process the result
			pTimedCallback->callback(true);
			CATAPULT_LOG(debug) << "read completed with: " << code;
			if (!pPacket)
				return;

			CATAPULT_LOG(debug) << "copying packet with size " << pPacket->Size << " and type " << pPacket->Type;
			buffer = CopyPacketToBuffer(*pPacket);
		});
	}
}}
