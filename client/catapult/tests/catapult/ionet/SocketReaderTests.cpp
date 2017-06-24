#include "catapult/ionet/SocketReader.h"
#include "catapult/ionet/BufferedPacketIo.h"
#include "catapult/ionet/IoTypes.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/thread/IoServiceThreadPool.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/net/ClientSocket.h"
#include "tests/test/net/SocketTestUtils.h"
#include "tests/test/nodeps/Atomics.h"
#include "tests/TestHarness.h"
#include <set>

namespace catapult { namespace ionet {

	namespace {
		struct SocketReadResult {
			std::vector<SocketOperationCode> CompletionCodes;
			size_t NumUnconsumedBytes;
		};

		std::shared_ptr<SocketReader> CreateSocketReader(
				boost::asio::io_service& service,
				const std::shared_ptr<PacketSocket>& pSocket,
				const ServerPacketHandlers& handlers) {
			auto pBufferedIo = CreateBufferedPacketIo(pSocket, boost::asio::strand(service));
			return ionet::CreateSocketReader(pSocket, pBufferedIo, handlers);
		}

		/// Creates server packet handlers that have a noop registered for the default packet type.
		ServerPacketHandlers CreateNoOpHandlers() {
			ServerPacketHandlers handlers;
			handlers.registerHandler(test::Default_Packet_Type, [](const auto&, const auto&) {});
			return handlers;
		}

		/// Starts a reader around \a service, \a pSocket, and \a handlers and sets up callbacks to
		/// update \a readResult.
		std::shared_ptr<SocketReader> StartReader(
				boost::asio::io_service& service,
				const std::shared_ptr<PacketSocket>& pSocket,
				const ServerPacketHandlers& handlers,
				SocketReadResult& readResult) {
			auto pReader = CreateSocketReader(service, pSocket, handlers);
			pReader->read([pSocket, &readResult](auto result) -> void {
				readResult.CompletionCodes.push_back(result);
				pSocket->stats([&readResult](const auto& stats) {
					readResult.NumUnconsumedBytes = stats.NumUnprocessedBytes;
				});
			});
			return pReader;
		}

		/// Writes all \a sendBuffers to a socket and reads them with a reader.
		std::pair<std::vector<ByteBuffer>, SocketReadResult> SendBuffers(
				const std::vector<ByteBuffer>& sendBuffers) {
			// Arrange: set up a packet handler that copies the received packet bytes into receivedBuffers
			ServerPacketHandlers handlers;
			std::vector<ByteBuffer> receivedBuffers;
			test::AddCopyBuffersHandler(handlers, receivedBuffers);

			// Act: "server" - reads packets from the socket using the reader
			//      "client" - writes sendBuffers to the socket
			SocketReadResult readResult;
			auto pPool = test::CreateStartedIoServiceThreadPool();
			std::shared_ptr<SocketReader> pReader;
			test::SpawnPacketServerWork(pPool->service(), [&](const auto& pServerSocket) -> void {
				pReader = StartReader(pPool->service(), pServerSocket, handlers, readResult);
			});
			test::AddClientWriteBuffersTask(pPool->service(), sendBuffers);

			// - wait for the test to complete
			pPool->join();
			return std::make_pair(receivedBuffers, readResult);
		}

		/// Sets a response packet in \a context with payload \a responseBytes.
		void RespondWithBytes(ServerPacketHandlerContext& context, const std::vector<uint8_t>& responseBytes) {
			auto numResponseBytes = static_cast<uint32_t>(responseBytes.size());
			auto pResponsePacket = CreateSharedPacket<Packet>(numResponseBytes);
			pResponsePacket->Size = sizeof(PacketHeader) + numResponseBytes;
			std::memcpy(pResponsePacket.get() + 1, responseBytes.data(), responseBytes.size());
			context.response(pResponsePacket);
		}

		void AssertSocketReadResult(
				const SocketReadResult& result,
				const std::vector<SocketOperationCode>& expectedCodes,
				size_t expectedNumUnconsumedBytes) {
			// Assert:
			EXPECT_EQ(expectedNumUnconsumedBytes, result.NumUnconsumedBytes);
			EXPECT_EQ(expectedCodes, result.CompletionCodes);
		}

		void AssertSocketReadSuccess(
				const SocketReadResult& result,
				size_t expectedNumReads,
				size_t expectedNumUnconsumedBytes) {
			// Assert: there should be `expectedNumReads` success codes followed by one insufficient data code,
			//         which indicates the end of a (successful) batch operation
			std::vector<SocketOperationCode> expectedCodes;
			for (auto i = 0u; i < expectedNumReads; ++i)
				expectedCodes.push_back(SocketOperationCode::Success);

			expectedCodes.push_back(SocketOperationCode::Insufficient_Data);
			AssertSocketReadResult(result, expectedCodes, expectedNumUnconsumedBytes);
		}

		void AssertSocketReadFailure(
				const SocketReadResult& result,
				size_t expectedNumReads,
				SocketOperationCode expectedCode,
				size_t expectedNumUnconsumedBytes) {
			// Assert: all reads prior to `expectedNumReads` should have succeeded, but the last
			//         read should have failed with `expectedCode` (the failure indicates the end
			//         of a (failed) batch operation)
			std::vector<SocketOperationCode> expectedCodes;
			for (auto i = 0u; i < expectedNumReads - 1; ++i)
				expectedCodes.push_back(SocketOperationCode::Success);

			expectedCodes.push_back(expectedCode);
			AssertSocketReadResult(result, expectedCodes, expectedNumUnconsumedBytes);
		}
	}

	TEST(SocketReaderTests, CanReadSinglePacket) {
		// Arrange: send a single buffer containing a single packet
		auto sendBuffer = test::GenerateRandomPacketBuffer(100, { 82, 75 });
		std::vector<ByteBuffer> sendBuffers{ sendBuffer };

		// Act:
		auto resultPair = SendBuffers(sendBuffers);
		const auto& receivedBuffers = resultPair.first;

		// Assert: the single packet was successfuly read
		AssertSocketReadSuccess(resultPair.second, 1, 18);
		ASSERT_EQ(1u, receivedBuffers.size());
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 0, 82u, receivedBuffers[0]);
	}

	TEST(SocketReaderTests, CanReadSinglePacketSpanningReads) {
		// Arrange: send a packet spanning three buffers
		auto sendBuffers = test::GenerateRandomPacketBuffers({ 50, 50, 50 });
		test::SetPacketAt(sendBuffers[0], 0, 125);
		test::SetPacketAt(sendBuffers[2], 25, 50);

		// Act:
		auto resultPair = SendBuffers(sendBuffers);
		const auto& receivedBuffers = resultPair.first;

		// Assert: the single packet was successfuly read
		AssertSocketReadSuccess(resultPair.second, 1, 25);
		ASSERT_EQ(1u, receivedBuffers.size());
		EXPECT_EQ(125u, receivedBuffers[0].size());
		EXPECT_EQ(test::ToHexString(sendBuffers[0]), test::ToHexString(&receivedBuffers[0][0], 50));
		EXPECT_EQ(test::ToHexString(sendBuffers[1]), test::ToHexString(&receivedBuffers[0][50], 50));
		EXPECT_EQ(test::ToHexString(&sendBuffers[2][0], 25), test::ToHexString(&receivedBuffers[0][100], 25));
	}

	TEST(SocketReaderTests, CanReadMultiplePacketsInSingleRead) {
		// Arrange: send a buffer containing three packets
		auto sendBuffer = test::GenerateRandomPacketBuffer(100, { 20, 17, 50, 25 });
		std::vector<ByteBuffer> sendBuffers{ sendBuffer };

		// Act:
		auto resultPair = SendBuffers(sendBuffers);
		const auto& receivedBuffers = resultPair.first;

		// Assert: the three packets were successfuly read (this is a result of using readMultiple)
		AssertSocketReadSuccess(resultPair.second, 3, 13);
		ASSERT_EQ(3u, receivedBuffers.size());
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 0, 20u, receivedBuffers[0]);
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 20, 17u, receivedBuffers[1]);
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 37, 50u, receivedBuffers[2]);
	}

	TEST(SocketReaderTests, CanRespondToPacket) {
		// Arrange: send a buffer containing three packets
		auto sendBuffer = test::GenerateRandomPacketBuffer(100, { 0x14, 0x11, 0x32, 0x19 });
		std::vector<ByteBuffer> sendBuffers{ sendBuffer };

		// - set up a packet handler that sends the sizes of all previously received packets back to the client
		//   as well as the total number of received packets
		ServerPacketHandlers handlers;
		std::vector<uint32_t> packetSizes;
		test::RegisterDefaultHandler(handlers, [&packetSizes](const auto& packet, auto& context) {
			packetSizes.push_back(packet.Size);

			std::vector<uint8_t> responseBytes{ static_cast<uint8_t>(packetSizes.size()) };
			for (auto size : packetSizes)
				responseBytes.push_back(static_cast<uint8_t>(size));

			RespondWithBytes(context, responseBytes);
		});

		// Act: "server" - reads packets from the socket using the reader
		//      "client" - writes sendBuffers to the socket and reads the response data into responseBytes
		SocketReadResult readResult;
		std::vector<uint8_t> responseBytes(3 * sizeof(Packet) + 9);
		auto pPool = test::CreateStartedIoServiceThreadPool();
		std::shared_ptr<SocketReader> pReader;
		test::SpawnPacketServerWork(pPool->service(), [&](const auto& pServerSocket) -> void {
			pReader = StartReader(pPool->service(), pServerSocket, handlers, readResult);
		});
		test::CreateClientSocket(pPool->service())->connect().then([&sendBuffers, &responseBytes](auto&& socketFuture) {
			auto pClientSocket = socketFuture.get();
			pClientSocket->write(sendBuffers).then([pClientSocket, &responseBytes](auto) {
				pClientSocket->read(responseBytes);
			});
		});

		// - wait for the test to complete
		pPool->join();

		// Assert:
		AssertSocketReadSuccess(readResult, 3, 13);
		auto expectedClientBytes = std::vector<uint8_t>{
			// packet 1
			0x0A, 0x00, 0x00, 0x00, // size
			0x00, 0x00, 0x00, 0x00, // type
			0x01, 0x14, // payload
			// packet 2
			0x0B, 0x00, 0x00, 0x00, // size
			0x00, 0x00, 0x00, 0x00, // type
			0x02, 0x14, 0x11, // payload
			// packet 3
			0x0C, 0x00, 0x00, 0x00, // size
			0x00, 0x00, 0x00, 0x00, // type
			0x03, 0x14, 0x11, 0x32 // payload
		};
		EXPECT_EQ(test::ToHexString(expectedClientBytes), test::ToHexString(responseBytes));
	}

	TEST(SocketReaderTests, ReadFailsOnReadError) {
		// Arrange: send one buffer containing one packet
		auto sendBuffer = test::GenerateRandomPacketBuffer(50, { 30, 70 });
		std::vector<ByteBuffer> sendBuffers{ sendBuffer };
		auto handlers = CreateNoOpHandlers();

		// Act: "server" - closes the socket and then reads packets from the socket using the reader
		//      "client" - writes sendBuffers to the socket
		SocketReadResult readResult;
		auto pPool = test::CreateStartedIoServiceThreadPool();
		std::shared_ptr<SocketReader> pReader;
		test::SpawnPacketServerWork(pPool->service(), [&](const auto& pServerSocket) -> void {
			pServerSocket->close();
			pReader = StartReader(pPool->service(), pServerSocket, handlers, readResult);
		});
		test::AddClientWriteBuffersTask(pPool->service(), sendBuffers);

		// - wait for the test to complete
		pPool->join();

		// Assert: the server should get a read error when attempting to read the buffer from the client
		//         (since the read failed, no bytes were read and none were consumed)
		AssertSocketReadFailure(readResult, 1, SocketOperationCode::Read_Error, 0);
	}

	TEST(SocketReaderTests, ReadFailsForUnknownPacket) {
		// Arrange: send one buffer containing one packet
		auto sendBuffer = test::GenerateRandomPacketBuffer(50, { 30, 70 });
		std::vector<ByteBuffer> sendBuffers{ sendBuffer };
		ServerPacketHandlers handlers;

		// Act: "server" - reads packets from the socket using the reader but does not have any handlers registered
		//      "client" - writes sendBuffers to the socket
		SocketReadResult readResult;
		auto pPool = test::CreateStartedIoServiceThreadPool();
		std::shared_ptr<SocketReader> pReader;
		test::SpawnPacketServerWork(pPool->service(), [&](const auto& pServerSocket) -> void {
			pReader = StartReader(pPool->service(), pServerSocket, handlers, readResult);
		});
		test::AddClientWriteBuffersTask(pPool->service(), sendBuffers);

		// - wait for the test to complete
		pPool->join();

		// Assert: the server treats the unknown packet as malformed
		//         (the 20 unconsumed bytes are the remainder in the buffer after consuming the first packet)
		AssertSocketReadFailure(readResult, 1, SocketOperationCode::Malformed_Data, 20);
	}

	TEST(SocketReaderTests, ReadFailsOnWriteError) {
		// Arrange: send one buffer containing one packet
		auto sendBuffer = test::GenerateRandomPacketBuffer(100, { 75, 50 });
		std::vector<ByteBuffer> sendBuffers{ sendBuffer };

		// Act: "server" - reads packets from the socket using the reader
		//               - closes the socket in the packet handler (before write) and then attempts to write
		//      "client" - writes sendBuffers to the socket
		auto pPool = test::CreateStartedIoServiceThreadPool();
		ServerPacketHandlers handlers;
		SocketReadResult readResult;
		std::shared_ptr<SocketReader> pReader;
		test::SpawnPacketServerWork(pPool->service(), [&](const auto& pServerSocket) -> void {
			test::RegisterDefaultHandler(handlers, [pServerSocket](const auto&, auto& context) {
				// - shut down the server socket
				pServerSocket->close();

				// - send a payload to the client to trigger a write
				CATAPULT_LOG(debug) << "sending byte to client";
				RespondWithBytes(context, {});
			});

			pReader = StartReader(pPool->service(), pServerSocket, handlers, readResult);
		});
		test::AddClientWriteBuffersTask(pPool->service(), sendBuffers);
		pPool->join();

		// Assert: the server should get a write error when attempting to write to the client
		//         (the 25 unconsumed bytes are the remainder in the buffer after consuming the first packet)
		AssertSocketReadFailure(readResult, 1, SocketOperationCode::Write_Error, 25);
	}

	TEST(SocketReaderTests, ReadFailsOnMalformedPacket) {
		// Arrange: send a buffer containing three packets with the second one malformed
		auto sendBuffer = test::GenerateRandomPacketBuffer(100, { 20, 0 });
		test::SetPacketAt(sendBuffer, 37, 50);
		std::vector<ByteBuffer> sendBuffers{ sendBuffer };

		// Act:
		auto resultPair = SendBuffers(sendBuffers);
		const auto& receivedBuffers = resultPair.first;

		// Assert: only the packet before the malformed one was read
		//         (the 80 unconsumed bytes are the remainder in the buffer after consuming the first packet)
		AssertSocketReadFailure(resultPair.second, 2, SocketOperationCode::Malformed_Data, 80);
		ASSERT_EQ(1u, receivedBuffers.size());
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 0, 20u, receivedBuffers[0]);
	}

	TEST(SocketReaderTests, CanChainReadOperations) {
		// Arrange: create two buffers
		auto sendBuffers = test::GenerateRandomPacketBuffers({ 20, 30 });

		// - set up a packet handler that copies the received packet bytes into receivedBuffers
		ServerPacketHandlers handlers;
		std::vector<ByteBuffer> receivedBuffers;
		test::AddCopyBuffersHandler(handlers, receivedBuffers);

		// Act: "server" - reads the first packet from the socket; signals; reads the second packet
		//      "client" - writes the first packet to the socket; waits; writes the second packet
		std::atomic_bool isFirstPacketRead(false);
		std::vector<std::vector<SocketOperationCode>> readResults(2);
		auto pPool = test::CreateStartedIoServiceThreadPool();
		std::shared_ptr<SocketReader> pReader;
		test::SpawnPacketServerWork(pPool->service(), [&](const auto& pServerSocket) -> void {
			pReader = CreateSocketReader(pPool->service(), pServerSocket, handlers);

			// - first read
			pReader->read([&readResults, &isFirstPacketRead, &pReader](auto result1) -> void {
				readResults[0].push_back(result1);

				// - only chain the second read on a termination condition
				if (SocketOperationCode::Success == result1)
					return;

				isFirstPacketRead = true;

				// - second read
				pReader->read([&readResults](auto result2) -> void {
					readResults[1].push_back(result2);
				});
			});
		});
		test::CreateClientSocket(pPool->service())->connect().then([&](auto&& socketFuture) {
			auto pClientSocket = socketFuture.get();
			pClientSocket->write(sendBuffers[0]).then([&, pClientSocket](auto) {
				WAIT_FOR(isFirstPacketRead);
				pClientSocket->write(sendBuffers[1]);
			});
		});

		// - wait for the test to complete
		pPool->join();

		// Assert: both buffers were read
		for (const auto& codes : readResults) {
			ASSERT_EQ(2u, codes.size());
			EXPECT_EQ(SocketOperationCode::Success, codes[0]);
			EXPECT_EQ(SocketOperationCode::Insufficient_Data, codes[1]);
		}

		ASSERT_EQ(2u, receivedBuffers.size());
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 0, 20u, receivedBuffers[0]);
		EXPECT_EQUAL_BUFFERS(sendBuffers[1], 0, 30u, receivedBuffers[1]);
	}

	TEST(SocketReaderTests, CanCloseDuringRead) {
		// Act: "server" - starts a read and closes the socket
		//      "client" - connects to the server
		auto handlers = CreateNoOpHandlers();
		SocketOperationCode readResult;
		auto pPool = test::CreateStartedIoServiceThreadPool();
		std::shared_ptr<SocketReader> pReader;
		test::SpawnPacketServerWork(pPool->service(), [&](const auto& pServerSocket) -> void {
			pReader = CreateSocketReader(pPool->service(), pServerSocket, handlers);
			pReader->read([&readResult](auto result) {
				readResult = result;
			});
			pServerSocket->close();
		});
		test::AddClientConnectionTask(pPool->service());

		// - wait for the test to complete
		pPool->join();

		// Assert: the socket was closed
		test::AssertSocketClosedDuringRead(readResult);
	}

	TEST(SocketReaderTests, CannotStartSimultaneousReads) {
		// Arrange: block the server handler until all reads have been attempted
		test::AutoSetFlag allReadsAttempted;
		ServerPacketHandlers handlers;
		handlers.registerHandler(test::Default_Packet_Type, [&](const auto&, const auto&) {
			allReadsAttempted.wait();
		});

		auto sendBuffers = test::GenerateRandomPacketBuffers({ 100 });

		// Act: "server" - starts multiple reads
		//      "client" - writes a buffer to the socket
		auto pPool = test::CreateStartedIoServiceThreadPool();
		std::shared_ptr<SocketReader> pReader;
		test::SpawnPacketServerWork(pPool->service(), [&](const auto& pServerSocket) -> void {
			pReader = CreateSocketReader(pPool->service(), pServerSocket, handlers);
			pReader->read([](auto) {});

			// Assert: attempting additional reads will throw
			EXPECT_THROW(pReader->read([](auto) {}), catapult_runtime_error);
			EXPECT_THROW(pReader->read([](auto) {}), catapult_runtime_error);
			allReadsAttempted.set();
		});
		test::AddClientWriteBuffersTask(pPool->service(), sendBuffers);

		// - wait for the test to complete
		pPool->join();
	}
}}
