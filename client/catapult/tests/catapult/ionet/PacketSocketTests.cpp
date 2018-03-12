#include "catapult/ionet/PacketSocket.h"
#include "catapult/ionet/IoTypes.h"
#include "catapult/ionet/Node.h"
#include "catapult/ionet/Packet.h"
#include "catapult/ionet/WorkingBuffer.h"
#include "catapult/thread/IoServiceThreadPool.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/net/ClientSocket.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/net/SocketTestUtils.h"

namespace catapult { namespace ionet {

#define TEST_CLASS PacketSocketTests

	// region write

	namespace {
		std::shared_ptr<Packet> CreateSmallWritePayload() {
			return test::BufferToPacket(test::GenerateRandomPacketBuffer(50));
		}

		std::shared_ptr<Packet> CreateLargeWritePayload() {
			return test::BufferToPacket(test::GenerateRandomPacketBuffer(1024 * 1024));
		}

		void AssertWriteSuccess(const PacketPayload& payload, const ByteBuffer& expectedPayload) {
			// Arrange: set up payloads
			const size_t Buffer_Size = payload.header().Size;
			ByteBuffer receiveBuffer(Buffer_Size);
			SocketOperationCode writeCode;

			// Act: "server" - writes a payload to the socket
			//      "client" - reads a payload from the socket
			auto pPool = test::CreateStartedIoServiceThreadPool();
			test::SpawnPacketServerWork(pPool->service(), [&payload, &writeCode](const auto& pServerSocket) {
				pServerSocket->write(payload, [&writeCode](auto code) {
					writeCode = code;
				});
			});
			test::AddClientReadBufferTask(pPool->service(), receiveBuffer);
			pPool->join();

			// Assert: the write succeeded and all data was read from the socket
			EXPECT_EQ(SocketOperationCode::Success, writeCode);
			EXPECT_EQUAL_BUFFERS(expectedPayload, 0, Buffer_Size, receiveBuffer);
		}
	}

	TEST(TEST_CLASS, WriteSucceedsWhenSocketWriteSucceeds_ZeroBufferPayload) {
		// Arrange: set up payloads
		auto packetBytes = test::GenerateRandomPacketBuffer(sizeof(Packet));
		auto payload = PacketPayload(test::BufferToPacket(packetBytes));

		// Sanity:
		EXPECT_EQ(sizeof(Packet), payload.header().Size);
		EXPECT_EQ(0u, payload.buffers().size());

		// Assert:
		AssertWriteSuccess(payload, packetBytes);
	}

	TEST(TEST_CLASS, WriteSucceedsWhenSocketWriteSucceeds_SingleBufferPayload) {
		// Arrange: set up payloads
		auto packetBytes = test::GenerateRandomPacketBuffer(50);
		auto payload = PacketPayload(test::BufferToPacket(packetBytes));

		// Sanity:
		EXPECT_EQ(50u, payload.header().Size);
		EXPECT_EQ(1u, payload.buffers().size());

		// Assert:
		AssertWriteSuccess(payload, packetBytes);
	}

	TEST(TEST_CLASS, WriteFailsWhenSocketWriteFails) {
		// Arrange: set up payloads
		auto pPayload = CreateSmallWritePayload();
		const size_t Buffer_Size = pPayload->Size;
		ByteBuffer receiveBuffer(Buffer_Size);
		SocketOperationCode writeCode;

		// Act: "server" - closes the socket and then writes a payload to the (closed) socket
		//      "client" - reads a payload from the socket
		auto pPool = test::CreateStartedIoServiceThreadPool();
		test::SpawnPacketServerWork(pPool->service(), [&pPayload, &writeCode](const auto& pServerSocket) {
			pServerSocket->close();
			CATAPULT_LOG(debug) << "closed server socket";

			pServerSocket->write(pPayload, [&writeCode](auto code) {
				writeCode = code;
			});
		});
		test::AddClientReadBufferTask(pPool->service(), receiveBuffer);
		pPool->join();

		// Assert:
		EXPECT_EQ(SocketOperationCode::Write_Error, writeCode);
		EXPECT_EQ(test::ToHexString(ByteBuffer(Buffer_Size)), test::ToHexString(receiveBuffer));
	}

	TEST(TEST_CLASS, WriteFailsWhenClientSocketCloses) {
		// Arrange: set up payloads
		auto payload = CreateLargeWritePayload();
		SocketOperationCode writeCode;
		std::atomic_bool isSocketClosed(false);

		// Act: "server" - waits for the client socket to close; writes to the socket
		//      "client" - closes the client socket
		auto pPool = test::CreateStartedIoServiceThreadPool();
		test::SpawnPacketServerWork(pPool->service(), [&payload, &writeCode, &isSocketClosed](const auto& pServerSocket) {
			WAIT_FOR(isSocketClosed);
			pServerSocket->write(payload, [&writeCode](auto code) {
				writeCode = code;
			});
		});
		test::CreateClientSocket(pPool->service())->connect().then([&isSocketClosed](auto&& socketFuture) {
			socketFuture.get()->shutdown();
			isSocketClosed = true;
		});
		pPool->join();

		// Assert:
		test::AssertSocketClosedDuringWrite(writeCode);
	}

	TEST(TEST_CLASS, WriteCanWriteMultipleConsecutivePayloads) {
		// Assert:
		test::AssertWriteCanWriteMultipleConsecutivePayloads([](const auto& pSocket) { return pSocket; });
	}

	// endregion

	// region read[Multiple]

	namespace {
		struct SendBuffersResult {
		public:
			SendBuffersResult() : NumHandlerCalls(0)
			{}

		public:
			SocketOperationCode Code;
			ByteBuffer ReceivedBuffer;
			size_t NumUnprocessedBytes;
			size_t NumHandlerCalls;
			bool IsPacketValid;
		};

		void FillResult(
				SendBuffersResult& result,
				const std::shared_ptr<PacketSocket>& pSocket,
				SocketOperationCode code,
				const Packet* pPacket) {
			++result.NumHandlerCalls;
			result.Code = code;
			result.IsPacketValid = nullptr != pPacket;
			if (result.IsPacketValid)
				result.ReceivedBuffer = test::CopyPacketToBuffer(*pPacket);

			pSocket->stats([pSocket, &result](const auto& stats) {
				result.NumUnprocessedBytes = stats.NumUnprocessedBytes;
			});
		}

		SendBuffersResult SendBuffers(const std::vector<ByteBuffer>& sendBuffers) {
			SendBuffersResult result;

			// Act: "server" - reads the next packet(s) from the socket (using read)
			//      "client" - sends all buffers to the socket
			auto pPool = test::CreateStartedIoServiceThreadPool();
			test::SpawnPacketServerWork(pPool->service(), [&result](const auto& pServerSocket) {
				pServerSocket->read([pServerSocket, &result](auto code, const auto* pPacket) {
					FillResult(result, pServerSocket, code, pPacket);
				});
			});
			test::AddClientWriteBuffersTask(pPool->service(), sendBuffers);
			pPool->join();

			return result;
		}

		void AssertSendBuffersResult(
				const SendBuffersResult& result,
				SocketOperationCode expectedCode,
				size_t expectedNumUnprocessedBytes) {
			// Assert:
			EXPECT_EQ(expectedCode, result.Code);
			EXPECT_EQ(expectedNumUnprocessedBytes, result.NumUnprocessedBytes);

			// - the handler should have been called once with a valid packet on success and nullptr on failure
			EXPECT_EQ(1u, result.NumHandlerCalls);
			EXPECT_EQ(SocketOperationCode::Success == expectedCode, result.IsPacketValid);
		}
	}

	TEST(TEST_CLASS, ReadCanProcessSinglePacket) {
		// Arrange: send a single buffer containing a single packet
		auto sendBuffer = test::GenerateRandomPacketBuffer(100, { 82 });
		std::vector<ByteBuffer> sendBuffers{ sendBuffer };

		// Act:
		auto result = SendBuffers(sendBuffers);
		const auto& receivedBuffer = result.ReceivedBuffer;

		// Assert:
		AssertSendBuffersResult(result, SocketOperationCode::Success, 18u);
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 0, 82u, receivedBuffer);
	}

	TEST(TEST_CLASS, ReadCanProcessSinglePacketSpanningReads) {
		// Arrange: send a packet spanning three buffers
		auto sendBuffers = test::GenerateRandomPacketBuffers({ 50, 50, 50 });
		test::SetPacketAt(sendBuffers[0], 0, 125);

		// Act:
		auto result = SendBuffers(sendBuffers);
		const auto& receivedBuffer = result.ReceivedBuffer;

		// Assert:
		AssertSendBuffersResult(result, SocketOperationCode::Success, 25u);
		EXPECT_EQ(125u, receivedBuffer.size());
		EXPECT_EQ(test::ToHexString(sendBuffers[0]), test::ToHexString(&receivedBuffer[0], 50));
		EXPECT_EQ(test::ToHexString(sendBuffers[1]), test::ToHexString(&receivedBuffer[50], 50));
		EXPECT_EQ(test::ToHexString(&sendBuffers[2][0], 25), test::ToHexString(&receivedBuffer[100], 25));
	}

	TEST(TEST_CLASS, ReadCanProcessFirstOfMultiplePacketsInSingleRead) {
		// Arrange: send a buffer containing three packets
		auto sendBuffer = test::GenerateRandomPacketBuffer(100, { 20, 17, 50 });
		std::vector<ByteBuffer> sendBuffers{ sendBuffer };

		// Act:
		auto result = SendBuffers(sendBuffers);
		const auto& receivedBuffer = result.ReceivedBuffer;

		// Assert: only the first buffer was returned
		AssertSendBuffersResult(result, SocketOperationCode::Success, 80u);
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 0, 20u, receivedBuffer);
	}

	TEST(TEST_CLASS, ReadFailsOnReadError) {
		// Arrange: send one buffer containing one packet
		auto sendBuffer = test::GenerateRandomPacketBuffer(50, { 30 });
		std::vector<ByteBuffer> sendBuffers{ sendBuffer };

		// Act: "server" - closes the server socket and then attempts to read the next packet(s) from the socket
		//      "client" - sends all buffers to the socket
		SendBuffersResult result;
		auto pPool = test::CreateStartedIoServiceThreadPool();
		test::SpawnPacketServerWork(pPool->service(), [&result](const auto& pServerSocket) {
			pServerSocket->close();
			CATAPULT_LOG(debug) << "closed server socket";

			pServerSocket->read([pServerSocket, &result](auto code, const auto* pPacket) {
				FillResult(result, pServerSocket, code, pPacket);
			});
		});
		test::AddClientWriteBuffersTask(pPool->service(), sendBuffers);
		pPool->join();

		// Assert: the server should get a read error when attempting to read from the socket
		//         (since nothing was read, the working buffer should be empty)
		AssertSendBuffersResult(result, SocketOperationCode::Read_Error, 0u);
	}

	TEST(TEST_CLASS, ReadFailsWhenClientSocketCloses) {
		// Arrange:
		SocketOperationCode readCode;
		std::atomic_bool isSocketClosed(false);

		// Act: "server" - waits for the client socket to close; reads from the socket
		//      "client" - closes the client socket
		auto pPool = test::CreateStartedIoServiceThreadPool();
		test::SpawnPacketServerWork(pPool->service(), [&readCode, &isSocketClosed](const auto& pServerSocket) {
			WAIT_FOR(isSocketClosed);
			pServerSocket->read([&readCode](auto code, const auto*) {
				readCode = code;
			});
		});
		test::CreateClientSocket(pPool->service())->connect().then([&isSocketClosed](auto&& socketFuture) {
			socketFuture.get()->shutdown();
			isSocketClosed = true;
		});
		pPool->join();

		// Assert:
		test::AssertSocketClosedDuringRead(readCode);
	}

	TEST(TEST_CLASS, ReadAbortsOnMalformedPacket) {
		// Arrange: send a buffer containing one malformed packet
		auto sendBuffer = test::GenerateRandomPacketBuffer(100, { 0 }); // malformed
		std::vector<ByteBuffer> sendBuffers{ sendBuffer };

		// Act:
		auto result = SendBuffers(sendBuffers);
		const auto& receivedBuffer = result.ReceivedBuffer;

		// Assert: the malformed data should not have been processed so nothing should have been removed from the
		//         working buffer
		AssertSendBuffersResult(result, SocketOperationCode::Malformed_Data, 100u);
		EXPECT_TRUE(receivedBuffer.empty());
	}

	namespace {
		std::vector<SendBuffersResult> SendBuffersMultiple(const std::vector<ByteBuffer>& sendBuffers) {
			std::vector<SendBuffersResult> results;
			results.reserve(10);

			// Act: "server" - reads the next packet(s) from the socket (using readMultiple)
			//      "client" - sends all buffers to the socket
			auto pPool = test::CreateStartedIoServiceThreadPool();
			test::SpawnPacketServerWork(pPool->service(), [&results](const auto& pServerSocket) {
				pServerSocket->readMultiple([pServerSocket, &results](auto code, const auto* pPacket) {
					results.push_back(SendBuffersResult());
					FillResult(results.back(), pServerSocket, code, pPacket);
				});
			});
			test::AddClientWriteBuffersTask(pPool->service(), sendBuffers);
			pPool->join();

			return results;
		}
	}

	TEST(TEST_CLASS, ReadMultipleCanProcessSinglePacket) {
		// Arrange: send a single buffer containing a single packet
		auto sendBuffer = test::GenerateRandomPacketBuffer(100, { 82, 40 });
		std::vector<ByteBuffer> sendBuffers{ sendBuffer };

		// Act:
		auto results = SendBuffersMultiple(sendBuffers);
		ASSERT_EQ(2u, results.size());
		const auto& receivedBuffer = results[0].ReceivedBuffer;

		// Assert:
		AssertSendBuffersResult(results[0], SocketOperationCode::Success, 18u);
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 0, 82u, receivedBuffer);

		AssertSendBuffersResult(results[1], SocketOperationCode::Insufficient_Data, 18u);
		EXPECT_TRUE(results[1].ReceivedBuffer.empty());
	}

	TEST(TEST_CLASS, ReadMultipleCanProcessSinglePacketSpanningReads) {
		// Arrange: send a packet spanning three buffers
		auto sendBuffers = test::GenerateRandomPacketBuffers({ 50, 50, 50 });
		test::SetPacketAt(sendBuffers[0], 0, 125);
		test::SetPacketAt(sendBuffers[2], 25, 125);

		// Act:
		auto results = SendBuffersMultiple(sendBuffers);
		ASSERT_EQ(2u, results.size());
		const auto& receivedBuffer = results[0].ReceivedBuffer;

		// Assert:
		AssertSendBuffersResult(results[0], SocketOperationCode::Success, 25u);
		EXPECT_EQ(125u, receivedBuffer.size());
		EXPECT_EQ(test::ToHexString(sendBuffers[0]), test::ToHexString(&receivedBuffer[0], 50));
		EXPECT_EQ(test::ToHexString(sendBuffers[1]), test::ToHexString(&receivedBuffer[50], 50));
		EXPECT_EQ(test::ToHexString(&sendBuffers[2][0], 25), test::ToHexString(&receivedBuffer[100], 25));

		AssertSendBuffersResult(results[1], SocketOperationCode::Insufficient_Data, 25u);
		EXPECT_TRUE(results[1].ReceivedBuffer.empty());
	}

	TEST(TEST_CLASS, ReadMultipleCanProcessMultiplePacketsInSingleRead) {
		// Arrange: send a buffer containing three packets
		auto sendBuffer = test::GenerateRandomPacketBuffer(100, { 20, 17, 50, 25 });
		std::vector<ByteBuffer> sendBuffers{ sendBuffer };

		// Act:
		auto results = SendBuffersMultiple(sendBuffers);

		// Assert: note that num unprocessed bytes is not updated to intermediate values because the entire
		//         readMultiple callback is happening on the socket strand, which prevents stats from being updated
		ASSERT_EQ(4u, results.size());
		AssertSendBuffersResult(results[0], SocketOperationCode::Success, 13u);
		AssertSendBuffersResult(results[1], SocketOperationCode::Success, 13u);
		AssertSendBuffersResult(results[2], SocketOperationCode::Success, 13u);

		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 0, 20u, results[0].ReceivedBuffer);
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 20, 17u, results[1].ReceivedBuffer);
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 37, 50u, results[2].ReceivedBuffer);

		AssertSendBuffersResult(results[3], SocketOperationCode::Insufficient_Data, 13u);
		EXPECT_TRUE(results[3].ReceivedBuffer.empty());
	}

	TEST(TEST_CLASS, ReadMultipleAbortsOnMalformedPacket) {
		// Arrange: send a buffer containing three packets with the second one malformed
		auto sendBuffer = test::GenerateRandomPacketBuffer(100, { 20, 0 });
		test::SetPacketAt(sendBuffer, 37, 50);
		std::vector<ByteBuffer> sendBuffers{ sendBuffer };

		// Act:
		auto results = SendBuffersMultiple(sendBuffers);

		// Assert: only the packet(s) before the malformed one was consumed
		//         (the 80 unconsumed bytes are the remainder in the buffer after consuming the first packet)
		EXPECT_EQ(2u, results.size());
		AssertSendBuffersResult(results[0], SocketOperationCode::Success, 80u);
		AssertSendBuffersResult(results[1], SocketOperationCode::Malformed_Data, 80u);

		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 0, 20u, results[0].ReceivedBuffer);
		EXPECT_TRUE(results[1].ReceivedBuffer.empty());
	}

	TEST(TEST_CLASS, ReadMultipleCanProcessMultiplePacketsSpanningMultipleReads) {
		// Arrange: send 2 packets spanning 4 buffers - packet 1 [1, 1, 0.5], packet 2 [0.5, 1]
		constexpr auto Size_Unit = 100u;
		constexpr auto Half_Size_Unit = Size_Unit / 2;
		auto sendBuffers = test::GenerateRandomPacketBuffers({ Size_Unit, Size_Unit, Size_Unit, Size_Unit });
		test::SetPacketAt(sendBuffers[0], 0, Size_Unit * 5 / 2);
		test::SetPacketAt(sendBuffers[2], Half_Size_Unit, Size_Unit * 3 / 2);

		// Act:
		std::vector<SendBuffersResult> results1;
		results1.reserve(2);
		std::vector<SendBuffersResult> results2;
		results2.reserve(2);

		// Act: "server" - chains two read multiple calls (each should read exactly one packet)
		//      "client" - sends two batches of buffers
		std::atomic_bool firstReadCompleted(false);
		auto pPool = test::CreateStartedIoServiceThreadPool();
		test::SpawnPacketServerWork(pPool->service(), [&](const auto& pServerSocket) {
			// - start the first read multiple call
			pServerSocket->readMultiple([&, pServerSocket](auto code, const auto* pPacket) {
				CATAPULT_LOG(debug) << "server handling readMultiple (1) callback " << code;
				results1.push_back(SendBuffersResult());
				FillResult(results1.back(), pServerSocket, code, pPacket);

				if (SocketOperationCode::Insufficient_Data != code)
					return;

				// - the first read multiple call completed, so start the second
				pServerSocket->readMultiple([&, pServerSocket](auto code2, const auto* pPacket2) {
					CATAPULT_LOG(debug) << "server handling readMultiple (2) callback " << code2;
					results2.push_back(SendBuffersResult());
					FillResult(results2.back(), pServerSocket, code2, pPacket2);
				});

				firstReadCompleted = true;
			});
		});
		test::CreateClientSocket(pPool->service())->connect()
			.then([&sendBuffers, &firstReadCompleted](auto&& socketFuture) {
				// - write 3 buffers (composed of one full and one partial packet) and wait for them to be read
				CATAPULT_LOG(debug) << "client writing three buffers";
				socketFuture.get()->write({ sendBuffers[0], sendBuffers[1], sendBuffers[2] });
				WAIT_FOR(firstReadCompleted);

				// - write the last buffer that completes the second packet
				CATAPULT_LOG(debug) << "client writing last buffer";
				socketFuture.get()->write({ sendBuffers[3] });
			});
		pPool->join();

		// Assert:
		ASSERT_EQ(2u, results1.size());
		AssertSendBuffersResult(results1[0], SocketOperationCode::Success, Half_Size_Unit);
		AssertSendBuffersResult(results1[1], SocketOperationCode::Insufficient_Data, Half_Size_Unit);

		ASSERT_EQ(2u, results2.size());
		AssertSendBuffersResult(results2[0], SocketOperationCode::Success, 0u);
		AssertSendBuffersResult(results2[1], SocketOperationCode::Insufficient_Data, 0u);

		const auto& receivedBuffer1 = results1[0].ReceivedBuffer;
		ASSERT_EQ(Size_Unit * 5 / 2, receivedBuffer1.size());
		EXPECT_EQ(test::ToHexString(sendBuffers[0]), test::ToHexString(&receivedBuffer1[0], Size_Unit));
		EXPECT_EQ(test::ToHexString(sendBuffers[1]), test::ToHexString(&receivedBuffer1[Size_Unit], Size_Unit));
		EXPECT_EQ(
				test::ToHexString(&sendBuffers[2][0], Half_Size_Unit),
				test::ToHexString(&receivedBuffer1[2 * Size_Unit], Half_Size_Unit));

		const auto& receivedBuffer2 = results2[0].ReceivedBuffer;
		ASSERT_EQ(Size_Unit * 3 / 2, receivedBuffer2.size());
		EXPECT_EQ(
				test::ToHexString(&sendBuffers[2][Half_Size_Unit], Half_Size_Unit),
				test::ToHexString(&receivedBuffer2[0], Half_Size_Unit));
		EXPECT_EQ(test::ToHexString(sendBuffers[3]), test::ToHexString(&receivedBuffer2[Half_Size_Unit], Size_Unit));
	}

	TEST(TEST_CLASS, ReadCanReadMultipleConsecutivePayloads) {
		// Assert:
		test::AssertReadCanReadMultipleConsecutivePayloads([](const auto& pSocket) { return pSocket; });
	}

	// endregion

	// region close

	TEST(TEST_CLASS, CloseTransitionsSocketToClosedState) {
		// Arrange:
		auto pPool = test::CreateStartedIoServiceThreadPool();
		auto pAcceptor = test::CreateImplicitlyClosedLocalHostAcceptor(pPool->service());

		// Act: "server" - accepts a connection and closes the socket
		//      "client" - connects to the server
		PacketSocket::Stats stats;
		std::shared_ptr<PacketSocket> pSocket;
		pPool->service().post([&acceptor = *pAcceptor, &stats, &pSocket]() {
			Accept(acceptor, test::CreatePacketSocketOptions(), [&stats, &pSocket](const auto& acceptedSocketInfo) {
				pSocket = acceptedSocketInfo.socket();
				pSocket->close();
				pSocket->stats([&stats](const auto& socketStats) { stats = socketStats; });
			});
		});
		test::AddClientConnectionTask(pPool->service());
		pPool->join();

		// Assert:
		EXPECT_TRUE(!!pSocket);
		EXPECT_FALSE(stats.IsOpen);
		EXPECT_EQ(0u, stats.NumUnprocessedBytes);
	}

	TEST(TEST_CLASS, CanCloseWithPendingOperations) {
		// Arrange: send one buffer containing one packet
		auto sendBuffer = test::GenerateRandomPacketBuffer(50, { 30 });
		std::vector<ByteBuffer> sendBuffers{ sendBuffer };

		// Act: "server" - start a read and destroy the server socket
		//      "client" - sends all buffers to the socket
		SendBuffersResult result;
		auto pPool = test::CreateStartedIoServiceThreadPool();
		test::SpawnPacketServerWork(pPool->service(), [&result](const auto& pServerSocket) {
			pServerSocket->read([&result](auto code, const auto* pPacket) {
				result.Code = code;
				result.IsPacketValid = nullptr != pPacket;
				if (result.IsPacketValid)
					result.ReceivedBuffer = test::CopyPacketToBuffer(*pPacket);
			});

			std::const_pointer_cast<PacketSocket>(pServerSocket).reset();
			CATAPULT_LOG(debug) << "destroyed server socket";
		});
		test::AddClientWriteBuffersTask(pPool->service(), sendBuffers);
		pPool->join();

		// Assert: the read should have completed successfully
		EXPECT_EQ(SocketOperationCode::Success, result.Code);
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 0, 30u, result.ReceivedBuffer);
	}

	// endregion

	// region synchronization

	TEST(TEST_CLASS, OperationsOnSingleSocketAreStranded) {
		// Arrange:
		auto pPool = test::CreateStartedIoServiceThreadPool();
		auto pAcceptor = test::CreateImplicitlyClosedLocalHostAcceptor(pPool->service());

		// Act: "server" - accepts a connection and posts a few operations on the socket
		//      "client" - connects to the server
		std::shared_ptr<PacketSocket> pSocket;
		std::vector<std::string> breadcrumbs;
		pPool->service().post([&acceptor = *pAcceptor, &breadcrumbs, &pSocket]() {
			Accept(acceptor, test::CreatePacketSocketOptions(), [&breadcrumbs, &pSocket](const auto& acceptedSocketInfo) {
				pSocket = acceptedSocketInfo.socket();
				auto addStatsBreadcrumb = [&breadcrumbs](const auto& stats) {
					breadcrumbs.push_back("open? " + std::to_string(stats.IsOpen));
				};

				for (auto i = 0u; i < 4u; ++i)
					pSocket->stats(addStatsBreadcrumb);

				pSocket->close();

				for (auto i = 0u; i < 4u; ++i)
					pSocket->stats(addStatsBreadcrumb);
			});
		});
		test::AddClientConnectionTask(pPool->service());
		pPool->join();

		// Assert: if the operations were not synchronized, the ordering would not be guaranteed (4 open, 4 closed)
		std::vector<std::string> expectedBreadcrumbs{
			"open? 1", "open? 1", "open? 1", "open? 1",
			"open? 0", "open? 0", "open? 0", "open? 0"
		};
		EXPECT_EQ(expectedBreadcrumbs, breadcrumbs);
	}

	// endregion

	// region Accept

	TEST(TEST_CLASS, AcceptReturnsOpenSocketOnSuccess) {
		// Arrange:
		auto pPool = test::CreateStartedIoServiceThreadPool();
		auto pAcceptor = test::CreateImplicitlyClosedLocalHostAcceptor(pPool->service());

		// Act: "server" - accepts a connection
		//      "client" - connects to the server
		PacketSocket::Stats stats;
		AcceptedPacketSocketInfo socketInfo;
		pPool->service().post([&acceptor = *pAcceptor, &stats, &socketInfo]() {
			Accept(acceptor, test::CreatePacketSocketOptions(), [&stats, &socketInfo](const auto& acceptedSocketInfo) {
				socketInfo = acceptedSocketInfo;
				socketInfo.socket()->stats([&stats](const auto& socketStats) { stats = socketStats; });
			});
		});
		test::AddClientConnectionTask(pPool->service());
		pPool->join();

		// Assert: loopback address is used in tests
		EXPECT_TRUE(!!socketInfo);
		EXPECT_EQ("127.0.0.1", socketInfo.host());
		EXPECT_TRUE(!!socketInfo.socket());

		EXPECT_TRUE(stats.IsOpen);
		EXPECT_EQ(0u, stats.NumUnprocessedBytes);
	}

	TEST(TEST_CLASS, AcceptReturnsNullSocketOnAcceptFailure) {
		// Arrange:
		auto pPool = test::CreateStartedIoServiceThreadPool();
		auto pAcceptor = test::CreateImplicitlyClosedLocalHostAcceptor(pPool->service());

		// Act: "server" - accepts a connection after invalidating the socket in the configure callback
		//      "client" - connects to the server
		AcceptedPacketSocketInfo socketInfo;
		std::atomic_bool isCallbackCalled(false);
		pPool->service().post([&acceptor = *pAcceptor, &socketInfo, &isCallbackCalled]() {
			auto invalidateSocket = [](auto& socket) { socket.open(boost::asio::ip::tcp::v4()); };
			auto options = test::CreatePacketSocketOptions();
			Accept(acceptor, options, invalidateSocket, [&socketInfo, &isCallbackCalled](const auto& acceptedSocketInfo) {
				socketInfo = acceptedSocketInfo;
				isCallbackCalled = true;
			});
		});
		test::AddClientConnectionTask(pPool->service());
		pPool->join();

		// Assert:
		EXPECT_FALSE(!!socketInfo);
		EXPECT_TRUE(socketInfo.host().empty());
		EXPECT_FALSE(!!socketInfo.socket());

		EXPECT_TRUE(isCallbackCalled);
	}

// disable for windows because getpeername appears to succeed even after socket is closed
#ifndef _WIN32

	TEST(TEST_CLASS, AcceptReturnsNullSocketOnRemoteEndpointFailure) {
		// Arrange: non-deterministic because timing dependent on server and client behavior
		//          this is a race condition that is difficult to reproduce, requiring:
		//          1. delay between async_accept success and remote_endpoint query
		//          2. client that sets SO_LINGER to 0s and immediately closes connection within (1) delay period
		test::RunNonDeterministicTest("remote endpoint failure", 50 * test::Max_Non_Deterministic_Test_Retries, [](auto) {
			auto pPool = test::CreateStartedIoServiceThreadPool(1);
			auto pAcceptor = test::CreateImplicitlyClosedLocalHostAcceptor(pPool->service());

			// Act: "server" - accepts a connection after invalidating the socket in the configure callback
			//      "client" - connects to the server and immediately aborts the connection
			AcceptedPacketSocketInfo socketInfo;
			std::atomic_bool isCallbackCalled(false);
			pPool->service().post([&acceptor = *pAcceptor, &socketInfo, &isCallbackCalled]() {
				Accept(acceptor, test::CreatePacketSocketOptions(), [&socketInfo, &isCallbackCalled](const auto& acceptedSocketInfo) {
					socketInfo = acceptedSocketInfo;
					isCallbackCalled = true;
				});
			});
			test::CreateClientSocket(pPool->service())->connect(test::ClientSocket::ConnectOptions::Abort);
			pPool->join();

			if (socketInfo)
				return false;

			// Assert:
			EXPECT_FALSE(!!socketInfo);
			EXPECT_TRUE(socketInfo.host().empty());
			EXPECT_FALSE(!!socketInfo.socket());

			EXPECT_TRUE(isCallbackCalled);
			return true;
		});
	}

#endif

	TEST(TEST_CLASS, AcceptCallsConfigureSocketCallbackForSocket) {
		// Arrange:
		auto pPool = test::CreateStartedIoServiceThreadPool();
		auto pAcceptor = test::CreateImplicitlyClosedLocalHostAcceptor(pPool->service());

		// Act: "server" - accepts a connection with a custom configure socket callback
		//      "client" - connects to the server
		auto numConfigureSocketCalls = 0u;
		pPool->service().post([&acceptor = *pAcceptor, &numConfigureSocketCalls]() {
			Accept(
					acceptor,
					test::CreatePacketSocketOptions(),
					[&numConfigureSocketCalls](const auto&) { ++numConfigureSocketCalls; },
					[](const auto&) {});
		});
		test::AddClientConnectionTask(pPool->service());
		pPool->join();

		// Assert: the configure socket callback should have been called
		EXPECT_EQ(1u, numConfigureSocketCalls);
	}

	TEST(TEST_CLASS, AcceptHonorsOptions) {
		// Arrange:
		constexpr auto Buffer_Size = 150u;
		auto sendBuffer = test::GenerateRandomPacketBuffer(Buffer_Size);
		auto pPool = test::CreateStartedIoServiceThreadPool();
		auto pAcceptor = test::CreateImplicitlyClosedLocalHostAcceptor(pPool->service());

		// Act: "server" - accepts a connection and reads a buffer
		//      "client" - connects to the server and writes a buffer
		SendBuffersResult result;
		auto options = test::CreatePacketSocketOptions();
		options.MaxPacketDataSize = Buffer_Size - sizeof(PacketHeader) - 1;
		pPool->service().post([&acceptor = *pAcceptor, &options, &result]() {
			Accept(acceptor, options, [&result](const auto& acceptedSocketInfo) {
				auto pServerSocket = acceptedSocketInfo.socket();
				pServerSocket->read([pServerSocket, &result](auto code, const auto* pPacket) {
					FillResult(result, pServerSocket, code, pPacket);
				});
			});
		});
		test::AddClientWriteBuffersTask(pPool->service(), { sendBuffer });
		pPool->join();

		// Assert: the packet should be flagged as malformed because it is too large
		AssertSendBuffersResult(result, SocketOperationCode::Malformed_Data, Buffer_Size);
	}

	// endregion

	// region Connect

	namespace {
		struct ConnectResultPair {
			ConnectResult Result;
			bool IsSocketValid;
		};

		auto Connect(boost::asio::io_service& service, const NodeEndpoint& endpoint, ConnectResultPair& result) {
			auto options = test::CreatePacketSocketOptions();
			return ionet::Connect(service, options, endpoint, [&result](auto connectResult, const auto& pSocket) {
				result.Result = connectResult;
				result.IsSocketValid = nullptr != pSocket;
			});
		}

		ConnectResultPair ConnectAndWait(boost::asio::io_service& service, const NodeEndpoint& endpoint) {
			// Act:
			ConnectResultPair result;
			Connect(service, endpoint, result);

			// - wait for all work to complete
			service.run();
			return result;
		}

		ConnectResultPair ConnectAndWait(const NodeEndpoint& endpoint) {
			boost::asio::io_service service;
			return ConnectAndWait(service, endpoint);
		}
	}

	TEST(TEST_CLASS, ConnectionFailsIfNodeEndpointCannotBeResolved) {
		// Act: attempt to connect to an invalid hostname
		auto result = ConnectAndWait({ "127.0.0.X", test::Local_Host_Port });

		// Assert:
#ifdef _WIN32
		constexpr ConnectResult Expected_Result = ConnectResult::Connect_Error;
#else
		constexpr ConnectResult Expected_Result = ConnectResult::Resolve_Error;
#endif

		EXPECT_EQ(Expected_Result, result.Result);
		EXPECT_FALSE(result.IsSocketValid);
	}

	TEST(TEST_CLASS, ConnectionFailsIfServerRefusesConnection) {
		// Act: attempt to connect to a local server that hasn't been started
		auto result = ConnectAndWait(test::CreateLocalHostNodeEndpoint());

		// Assert:
		EXPECT_EQ(ConnectResult::Connect_Error, result.Result);
		EXPECT_FALSE(result.IsSocketValid);
	}

	namespace {
		void AssertCanConnectToServer(boost::asio::io_service& service, const NodeEndpoint& endpoint) {
			// Act: attempt to connect to a running server
			auto result = ConnectAndWait(service, endpoint);

			// Assert:
			EXPECT_EQ(ConnectResult::Connected, result.Result);
			EXPECT_TRUE(result.IsSocketValid);
		}
	}

	TEST(TEST_CLASS, ConnectionSucceedsIfServerAcceptsConnection) {
		// Arrange: set up a server acceptor thread
		boost::asio::io_service service;
		auto pAcceptor = test::CreateImplicitlyClosedLocalHostAcceptor(service);

		// Assert:
		AssertCanConnectToServer(service, test::CreateLocalHostNodeEndpoint());
	}

	namespace {
		bool RunCancellationTestIteration() {
			// Arrange: set up a server thread
			auto pPool = test::CreateStartedIoServiceThreadPool();
			auto& service = pPool->service();

			CATAPULT_LOG(debug) << "starting async accept";
			auto pAcceptor = test::CreateImplicitlyClosedLocalHostAcceptor(service);
			auto acceptorStrand = boost::asio::strand(service);
			auto serverSocket = boost::asio::ip::tcp::socket(service);

			// - post an async server accept
			acceptorStrand.post([&acceptor = *pAcceptor, &serverSocket]() {
				acceptor.async_accept(serverSocket, [](const auto& acceptEc) {
					CATAPULT_LOG(debug) << "async_accept completed with: " << acceptEc.message();
				});
			});

			// Act: attempt to connect to a running local server
			ConnectResultPair result;
			auto cancel = Connect(service, test::CreateLocalHostNodeEndpoint(), result);

			// - immediately cancel the connect
			cancel();

			// - cancel any pending server accepts (depending on timing, either the resolve or connect could be cancelled)
			acceptorStrand.post([&acceptor = *pAcceptor]() {
				acceptor.cancel();
			});

			// - wait for the pool to finish
			pPool->join();

			// Retry: if the socket connected too quickly (before it was cancelled)
			if (ConnectResult::Connected == result.Result) {
				CATAPULT_LOG(warning) << "Socket was connected before cancellation";
				return false;
			}

			// Assert: the connect was cancelled
			EXPECT_EQ(ConnectResult::Connect_Cancelled, result.Result);
			EXPECT_FALSE(result.IsSocketValid);
			return true;
		}
	}

	TEST(TEST_CLASS, ConnectionFailsIfConnectAttemptIsCancelled) {
		// Assert: non-deterministic because a socket could connect before it is cancelled
		test::RunNonDeterministicTest("Cancellation", RunCancellationTestIteration);
	}

	TEST(TEST_CLASS, ConnectHonorsOptions) {
		// Arrange:
		constexpr auto Buffer_Size = 150u;
		auto sendBuffer = test::GenerateRandomPacketBuffer(Buffer_Size);
		auto pPool = test::CreateStartedIoServiceThreadPool();

		// Act: "server" - accepts a connection and writes a buffer
		//      "client" - connects to the server and reads a buffer
		SendBuffersResult result;
		test::SpawnPacketServerWork(pPool->service(), [&sendBuffer](const auto& pServerSocket) {
			pServerSocket->write(test::BufferToPacket(sendBuffer), [](auto code) {
				CATAPULT_LOG(debug) << "write completed with code " << code;
			});
		});

		auto endpoint = test::CreateLocalHostNodeEndpoint();
		auto options = test::CreatePacketSocketOptions();
		options.MaxPacketDataSize = Buffer_Size - sizeof(PacketHeader) - 1;
		ionet::Connect(pPool->service(), options, endpoint, [&result](auto, const auto& pClientSocket) {
			pClientSocket->read([pClientSocket, &result](auto code, const auto* pPacket) {
				CATAPULT_LOG(debug) << "read completed with code " << code;
				FillResult(result, pClientSocket, code, pPacket);
			});
		});
		pPool->join();

		// Assert: the packet should be flagged as malformed because it is too large
		//         do not use AssertSendBuffersResult beacause the number of unprocessed bytes is non-deterministic
		//         either the packet header or the entire packet will have been read (the header data marks the packet as malformed)
		EXPECT_EQ(SocketOperationCode::Malformed_Data, result.Code);
		EXPECT_LE(sizeof(PacketHeader), result.NumUnprocessedBytes);
		EXPECT_EQ(1u, result.NumHandlerCalls);
		EXPECT_FALSE(result.IsPacketValid);
	}

	// endregion
}}
