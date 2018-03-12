#include "NetworkTestUtils.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/net/PacketWriters.h"
#include "catapult/net/VerifyPeer.h"
#include "catapult/thread/TimedCallback.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/net/SocketTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	std::shared_ptr<ionet::PacketSocket> ConnectToLocalHost(
			boost::asio::io_service& service,
			unsigned short port,
			const Key& serverPublicKey) {
		// Act: connect to the server
		std::atomic_bool isConnected(false);
		auto options = CreatePacketSocketOptions();
		auto endpoint = CreateLocalHostNodeEndpoint(port);
		auto clientKeyPair = GenerateKeyPair();
		std::shared_ptr<ionet::PacketSocket> pIo;
		ionet::Connect(service, options, endpoint, [&](auto connectResult, const auto& pConnectedSocket) {
			CATAPULT_LOG(debug) << "node is connected with code " << connectResult;
			pIo = pConnectedSocket;
			if (!pIo)
				return;

			net::VerifyServer(pIo, serverPublicKey, clientKeyPair, [&isConnected](auto verifyResult, const auto&) {
				CATAPULT_LOG(debug) << "node verified with result " << verifyResult;
				if (net::VerifyResult::Success == verifyResult)
					isConnected = true;
			});
		});
		WAIT_FOR(isConnected);
		return pIo;
	}

	void ConnectToLocalHost(net::PacketWriters& packetWriters, const Key& serverPublicKey) {
		// Act: connect to the server
		net::PeerConnectResult connectResult;
		std::atomic<size_t> numConnects(0);
		packetWriters.connect(CreateLocalHostNode(serverPublicKey), [&](auto result) {
			connectResult = result;
			++numConnects;
		});
		WAIT_FOR_ONE(numConnects);

		// Assert: a single connection was accepted
		EXPECT_EQ(net::PeerConnectResult::Accepted, connectResult);
		EXPECT_EQ(1u, numConnects);
	}

	void AsyncReadIntoBuffer(boost::asio::io_service& service, ionet::PacketSocket& io, ionet::ByteBuffer& buffer) {
		// set up a timer that will close the socket if it takes too long to respond to read
		// a TimedCallback can't be passed directly to io.read because the TimedCallback would be called by the socket handler and
		// then dispatch to a separate thread (to execute in context of the TimedCallback strand); this switching would allow the
		// socket read handler to potentially complete (and invalidate the read packet pointer) before the user callback is called
		auto pTimedCallback = thread::MakeTimedCallback(service, consumer<bool>([](auto) {}), false);
		pTimedCallback->setTimeout(utils::TimeSpan::FromSeconds(detail::Default_Wait_Timeout));
		pTimedCallback->setTimeoutHandler([&io]() {
			CATAPULT_LOG(warning) << "closing socket due to timeout";
			io.close();
		});

		io.read([&io, &buffer, pTimedCallback](auto code, const auto* pPacket) {
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
