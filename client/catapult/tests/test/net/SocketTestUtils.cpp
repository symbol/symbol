#include "SocketTestUtils.h"
#include "ClientSocket.h"
#include "NodeTestUtils.h"
#include "catapult/crypto/Signer.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/net/Challenge.h"
#include "catapult/net/ConnectionSettings.h"
#include "catapult/thread/IoServiceThreadPool.h"
#include "tests/test/core/ThreadPoolTestUtils.h"

namespace catapult { namespace test {

	const boost::asio::ip::tcp::endpoint Local_Host = CreateLocalHostEndpoint(Local_Host_Port);

	boost::asio::ip::tcp::endpoint CreateLocalHostEndpoint(unsigned short port) {
		return boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), port);
	}

	ionet::PacketSocketOptions CreatePacketSocketOptions() {
		return net::ConnectionSettings().toSocketOptions();
	}

	namespace {
		void EnableAddressReuse(boost::asio::ip::tcp::acceptor& acceptor) {
			acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));

#ifndef _WIN32
			using reuse_port = boost::asio::detail::socket_option::boolean<BOOST_ASIO_OS_DEF(SOL_SOCKET), SO_REUSEPORT>;
			acceptor.set_option(reuse_port(true));
#endif
		}

		void BindAcceptor(boost::asio::ip::tcp::acceptor& acceptor, const boost::asio::ip::tcp::endpoint& endpoint) {
			acceptor.open(endpoint.protocol());
			EnableAddressReuse(acceptor);
			acceptor.bind(endpoint);
		}
	}

	std::shared_ptr<boost::asio::ip::tcp::acceptor> CreateLocalHostAcceptor(boost::asio::io_service& service) {
		auto pAcceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(service);
		BindAcceptor(*pAcceptor, Local_Host);
		pAcceptor->listen();
		return std::shared_ptr<boost::asio::ip::tcp::acceptor>(pAcceptor.get(), [&service, pAcceptor](const auto*) {
			// prevent calling close from async_accept handler (where reference count usually goes to zero)
			// to mitigate MacOS race condition (and hang) when kevent and close are called by different threads
			service.post([pAcceptor]() {
				CATAPULT_LOG(debug) << "closing socket acceptor";
				pAcceptor->close();
				CATAPULT_LOG(debug) << "socket acceptor closed";
			});
		});
	}

	void SpawnPacketServerWork(boost::asio::io_service& service, const PacketSocketWork& serverWork) {
		// Arrange: start listening immediately
		//          this will prevent a race condition where the client work (async_connect) is dispatched before
		//          the server work has set up a listener (listen), resulting in an unwanted connection
		//          refused error
		CATAPULT_LOG(debug) << "starting server listening on " << Local_Host;
		auto pAcceptor = CreateLocalHostAcceptor(service);

		// - post the work to the threadpool
		service.post([&service, serverWork, pAcceptor]() {
			// Arrange: accept a connection
			ionet::Accept(*pAcceptor, CreatePacketSocketOptions(), [serverWork, pAcceptor](const auto& pSocket) {
				CATAPULT_LOG(debug) << "server socket accepted: " << pSocket.get();

				// Act: perform the server work
				serverWork(pSocket);
			});
		});
	}

	void SpawnPacketClientWork(boost::asio::io_service& service, const PacketSocketWork& clientWork) {
		service.post([&service, clientWork]() {
			auto endpoint = CreateLocalHostNodeEndpoint();
			ionet::Connect(service, CreatePacketSocketOptions(), endpoint, [clientWork](auto result, const auto& pSocket) {
				CATAPULT_LOG(debug) << "client socket connected " << result;
				clientWork(pSocket);
			});
		});
	}

	bool IsSocketOpen(ionet::PacketSocket& socket) {
		ionet::PacketSocket::Stats stats;
		std::atomic_bool hasStats(false);
		socket.stats([&](const auto& socketStats) {
			stats = socketStats;
			hasStats = true;
		});

		WAIT_FOR(hasStats);
		return stats.IsOpen;
	}

	// *** write ***

	namespace {
		const size_t Large_Buffer_Size = 100 * 1024;

		struct LargeWritePayload {
		public:
			explicit LargeWritePayload(uint32_t bufferSize)
					: Buffer(GenerateRandomPacketBuffer(bufferSize))
					, pPacket(BufferToPacket(Buffer))
			{}

		public:
			const ionet::ByteBuffer Buffer;
			const std::shared_ptr<ionet::Packet> pPacket;
			ionet::SocketOperationCode Result;

		public:
			template<typename T>
			bool isBufferEqualTo(T start, T end) const {
				return std::equal(Buffer.cbegin(), Buffer.cend(), start, end);
			}
		};
	}

	void AssertWriteCanWriteMultipleConsecutivePayloads(const PacketIoTransform& transform) {
		// Arrange: set up payloads
		LargeWritePayload payload1(Large_Buffer_Size);
		LargeWritePayload payload2(Large_Buffer_Size);
		ionet::ByteBuffer receiveBuffer(2 * Large_Buffer_Size);

		// Act: "server" - starts two chained async write operations
		//      "client" - reads a payload from the socket
		auto pPool = CreateStartedIoServiceThreadPool();
		SpawnPacketServerWork(pPool->service(), [&](const auto& pServerSocket) -> void {
			auto pIo = transform(pPool->service(), pServerSocket);
			pIo->write(payload1.pPacket, [pIo, &payload1, &payload2](auto result1) -> void {
				payload1.Result = result1;

				pIo->write(payload2.pPacket, [&payload2](auto result2) {
					payload2.Result = result2;
				});
			});
		});
		AddClientReadBufferTask(pPool->service(), receiveBuffer);
		pPool->join();

		// Assert: both writes should have succeeded and no data should have been interleaved
		EXPECT_EQ(ionet::SocketOperationCode::Success, payload1.Result);
		EXPECT_EQ(ionet::SocketOperationCode::Success, payload2.Result);

		auto pReceiveBuffer1End = receiveBuffer.cbegin() + Large_Buffer_Size;
		EXPECT_TRUE(payload1.isBufferEqualTo(receiveBuffer.cbegin(), pReceiveBuffer1End));
		EXPECT_TRUE(payload2.isBufferEqualTo(pReceiveBuffer1End, receiveBuffer.cend()));
	}

	void AssertWriteCanWriteMultipleSimultaneousPayloadsWithoutInterleaving(const PacketIoTransform& transform) {
		// Arrange: set up payloads
		LargeWritePayload payload1(Large_Buffer_Size);
		LargeWritePayload payload2(Large_Buffer_Size);
		ionet::ByteBuffer receiveBuffer(2 * Large_Buffer_Size);

		// Act: "server" - starts two concurrent async write operations
		//      "client" - reads a payload from the socket
		auto pPool = CreateStartedIoServiceThreadPool();
		SpawnPacketServerWork(pPool->service(), [&](const auto& pServerSocket) -> void {
			auto pIo = transform(pPool->service(), pServerSocket);
			pIo->write(payload1.pPacket, [&payload1](auto result) {
				payload1.Result = result;
			});
			pIo->write(payload2.pPacket, [&payload2](auto result) {
				payload2.Result = result;
			});
		});
		AddClientReadBufferTask(pPool->service(), receiveBuffer);
		pPool->join();

		// Assert: both writes should have succeeded and no data should have been interleaved
		EXPECT_EQ(ionet::SocketOperationCode::Success, payload1.Result);
		EXPECT_EQ(ionet::SocketOperationCode::Success, payload2.Result);

		auto pReceiveBuffer1End = receiveBuffer.cbegin() + Large_Buffer_Size;
		EXPECT_TRUE(payload1.isBufferEqualTo(receiveBuffer.cbegin(), pReceiveBuffer1End));
		EXPECT_TRUE(payload2.isBufferEqualTo(pReceiveBuffer1End, receiveBuffer.cend()));
	}

	/** read **/

	namespace {
		struct LargeReadPayload {
		public:
			explicit LargeReadPayload(uint32_t bufferSize) : Buffer(GenerateRandomPacketBuffer(bufferSize))
			{}

		public:
			const ionet::ByteBuffer Buffer;
			ionet::ByteBuffer ReadBuffer;
			ionet::SocketOperationCode Result;

		public:
			void update(const ionet::SocketOperationCode& result, const ionet::Packet* pPacket) {
				Result = result;
				if (!pPacket)
					return;

				ReadBuffer = CopyPacketToBuffer(*pPacket);
			}

			bool isBufferRead() const {
				return std::equal(Buffer.cbegin(), Buffer.cend(), ReadBuffer.cbegin(), ReadBuffer.cend());
			}
		};

		void AddClientWriteBuffersTask(
				boost::asio::io_service& service,
				const LargeReadPayload& payload1,
				const LargeReadPayload& payload2) {
			test::AddClientWriteBuffersTask(service, { payload1.Buffer, payload2.Buffer });
		}
	}

	void AssertReadCanReadMultipleConsecutivePayloads(const PacketIoTransform& transform) {
		// Arrange: set up payloads
		LargeReadPayload payload1(Large_Buffer_Size);
		LargeReadPayload payload2(Large_Buffer_Size);

		// Act: "server" - starts two chained async read operations
		//      "client" - writes the payloads to the socket
		auto pPool = CreateStartedIoServiceThreadPool();
		SpawnPacketServerWork(pPool->service(), [&](const auto& pServerSocket) -> void {
			auto pIo = transform(pPool->service(), pServerSocket);
			pIo->read([pIo, &payload1, &payload2](auto result1, const auto* pPacket1) -> void {
				payload1.update(result1, pPacket1);

				pIo->read([&payload2](auto result2, const auto* pPacket2) {
					payload2.update(result2, pPacket2);
				});
			});
		});
		AddClientWriteBuffersTask(pPool->service(), payload1, payload2);
		pPool->join();

		// Assert: both reads should have succeeded and no data should have been interleaved
		EXPECT_EQ(ionet::SocketOperationCode::Success, payload1.Result);
		EXPECT_EQ(ionet::SocketOperationCode::Success, payload2.Result);

		EXPECT_TRUE(payload1.isBufferRead());
		EXPECT_TRUE(payload2.isBufferRead());
	}

	void AssertReadCanReadMultipleSimultaneousPayloadsWithoutInterleaving(const PacketIoTransform& transform) {
		// Arrange: set up payloads
		LargeReadPayload payload1(Large_Buffer_Size);
		LargeReadPayload payload2(Large_Buffer_Size);

		// Act: "server" - starts two concurrent async read operations
		//      "client" - writes the payloads to the socket
		auto pPool = CreateStartedIoServiceThreadPool();
		SpawnPacketServerWork(pPool->service(), [&](const auto& pServerSocket) -> void {
			auto pIo = transform(pPool->service(), pServerSocket);
			pIo->read([&payload1](auto result, const auto* pPacket) {
				payload1.update(result, pPacket);
			});
			pIo->read([&payload2](auto result, const auto* pPacket) {
				payload2.update(result, pPacket);
			});
		});
		AddClientWriteBuffersTask(pPool->service(), payload1, payload2);
		pPool->join();

		// Assert: both reads should have succeeded and no data should have been interleaved
		EXPECT_EQ(ionet::SocketOperationCode::Success, payload1.Result);
		EXPECT_EQ(ionet::SocketOperationCode::Success, payload2.Result);

		EXPECT_TRUE(payload1.isBufferRead());
		EXPECT_TRUE(payload2.isBufferRead());
	}

	void AssertSocketClosedDuringRead(const ionet::SocketOperationCode& readResult) {
		// Assert: the socket was closed
		//         the underlying socket can either return eof (Closed) or operation cancelled (Read_Error)
		std::set<ionet::SocketOperationCode> possibleValues{
			ionet::SocketOperationCode::Closed,
			ionet::SocketOperationCode::Read_Error
		};
		EXPECT_TRUE(possibleValues.end() != possibleValues.find(readResult)) << "read result: " << readResult;
	}

	void AssertSocketClosedDuringWrite(const ionet::SocketOperationCode& writeResult) {
		// Assert: the socket was closed
		//         the underlying socket can return operation cancelled (Write_Error)
		EXPECT_EQ(ionet::SocketOperationCode::Write_Error, writeResult);
	}
}}
