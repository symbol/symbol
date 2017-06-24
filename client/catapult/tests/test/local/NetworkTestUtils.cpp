#include "NetworkTestUtils.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/net/PacketWriters.h"
#include "catapult/net/VerifyPeer.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/net/SocketTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	std::shared_ptr<ionet::PacketIo> ConnectToLocalHost(
			boost::asio::io_service& service,
			unsigned short port,
			const Key& serverPublicKey) {
		// Act: connect to the server
		std::atomic_bool isConnected(false);
		auto options = CreatePacketSocketOptions();
		auto endpoint = CreateLocalHostNodeEndpoint(port);
		auto clientKeyPair = GenerateKeyPair();
		std::shared_ptr<ionet::PacketIo> pIo;
		ionet::Connect(service, options, endpoint, [&](auto connectResult, const auto& pConnectedSocket) -> void {
			CATAPULT_LOG(debug) << "node is connected with code " << connectResult;
			pIo = pConnectedSocket;

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

	void AsyncReadIntoBuffer(ionet::PacketIo& io, ionet::ByteBuffer& buffer) {
		io.read([&buffer](auto code, const auto* pPacket) {
			CATAPULT_LOG(debug) << "read completed with: " << code;
			if (!pPacket)
				return;

			buffer = CopyPacketToBuffer(*pPacket);
		});
	}

	// region DefaultPacketWritersHolder

	struct DefaultPacketWritersHolder::DefaultPacketWritersHolder::Impl {
	public:
		Impl()
				: KeyPair(GenerateKeyPair())
				, pPacketWriters(net::CreatePacketWriters(CreateStartedIoServiceThreadPool(1), KeyPair, net::ConnectionSettings()))
		{}

	public:
		crypto::KeyPair KeyPair;
		std::shared_ptr<net::PacketWriters> pPacketWriters;
	};

	DefaultPacketWritersHolder::DefaultPacketWritersHolder() : m_pImpl(std::make_unique<Impl>())
	{}

	DefaultPacketWritersHolder::~DefaultPacketWritersHolder() = default;

	net::PacketWriters& DefaultPacketWritersHolder::get() {
		return *m_pImpl->pPacketWriters;
	}

	// endregion
}}
