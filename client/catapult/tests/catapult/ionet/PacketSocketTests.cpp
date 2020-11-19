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

#include "catapult/ionet/PacketSocket.h"
#include "catapult/ionet/IoTypes.h"
#include "catapult/ionet/Node.h"
#include "catapult/ionet/Packet.h"
#include "catapult/ionet/WorkingBuffer.h"
#include "catapult/thread/IoThreadPool.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/core/mocks/MockPacketSocket.h"
#include "tests/test/net/ClientSocket.h"
#include "tests/test/net/NodeTestUtils.h"
#include "tests/test/net/SocketTestUtils.h"

namespace catapult { namespace ionet {

#define TEST_CLASS PacketSocketTests

	// region write

	namespace {
		PacketPayload CreateSmallWritePayload() {
			return test::BufferToPacketPayload(test::GenerateRandomPacketBuffer(50));
		}

		PacketPayload CreateLargeWritePayload() {
			return test::BufferToPacketPayload(test::GenerateRandomPacketBuffer(1024 * 1024));
		}

		void AssertWriteSuccess(const PacketPayload& payload, const ByteBuffer& expectedPayload, uint32_t maxPacketDataSize = 0) {
			// Arrange:
			auto options = test::CreatePacketSocketOptions();
			if (0 != maxPacketDataSize)
				options.MaxPacketDataSize = maxPacketDataSize;

			// - set up payloads
			auto bufferSize = payload.header().Size;
			ByteBuffer receiveBuffer(bufferSize);
			SocketOperationCode writeCode;

			// Act: "server" - writes a payload to the socket
			//      "client" - reads a payload from the socket
			auto pPool = test::CreateStartedIoThreadPool();
			test::SpawnPacketServerWork(pPool->ioContext(), options, [&payload, &writeCode](const auto& pServerSocket) {
				pServerSocket->write(payload, [&writeCode](auto code) {
					writeCode = code;
				});
			});
			auto pClientSocket = test::AddClientReadBufferTask(pPool->ioContext(), receiveBuffer);
			pPool->join();

			// Assert: the write succeeded and all data was read from the socket
			EXPECT_EQ(SocketOperationCode::Success, writeCode);
			EXPECT_EQUAL_BUFFERS(expectedPayload, 0, bufferSize, receiveBuffer);
		}

		void AssertWriteFailure(const PacketPayload& payload, uint32_t maxPacketDataSize) {
			// Arrange:
			auto options = test::CreatePacketSocketOptions();
			options.MaxPacketDataSize = maxPacketDataSize;

			SocketOperationCode writeCode;

			// Act: "server" - writes a payload to the socket
			//      "client" - accepts a connection
			auto pPool = test::CreateStartedIoThreadPool();
			test::SpawnPacketServerWork(pPool->ioContext(), options, [&payload, &writeCode](const auto& pServerSocket) {
				pServerSocket->write(payload, [&writeCode](auto code) {
					writeCode = code;
				});
			});
			auto pClientSocket = test::AddClientConnectionTask(pPool->ioContext());
			pPool->join();

			// Assert: the write failed due to malformed data
			EXPECT_EQ(SocketOperationCode::Malformed_Data, writeCode);
		}
	}

	TEST(TEST_CLASS, WriteSucceedsWhenSocketWriteSucceeds_ZeroBufferPayload) {
		// Arrange: set up payloads
		auto packetBytes = test::GenerateRandomPacketBuffer(sizeof(Packet));
		auto payload = test::BufferToPacketPayload(packetBytes);

		// Sanity:
		EXPECT_EQ(sizeof(Packet), payload.header().Size);
		EXPECT_EQ(0u, payload.buffers().size());

		// Assert:
		AssertWriteSuccess(payload, packetBytes);
	}

	TEST(TEST_CLASS, WriteSucceedsWhenSocketWriteSucceeds_SingleBufferPayload) {
		// Arrange: set up payloads
		auto packetBytes = test::GenerateRandomPacketBuffer(50);
		auto payload = test::BufferToPacketPayload(packetBytes);

		// Sanity:
		EXPECT_EQ(50u, payload.header().Size);
		EXPECT_EQ(1u, payload.buffers().size());

		// Assert:
		AssertWriteSuccess(payload, packetBytes);
	}

	TEST(TEST_CLASS, WriteFailsWhenSocketWriteFails) {
		// Arrange: set up payloads
		auto payload = CreateSmallWritePayload();
		auto bufferSize = payload.header().Size;
		ByteBuffer receiveBuffer(bufferSize);
		SocketOperationCode writeCode;

		// Act: "server" - closes the socket and then writes a payload to the (closed) socket
		//      "client" - reads a payload from the socket
		auto pPool = test::CreateStartedIoThreadPool();
		test::SpawnPacketServerWork(pPool->ioContext(), [&payload, &writeCode](const auto& pServerSocket) {
			pServerSocket->close();
			CATAPULT_LOG(debug) << "closed server socket";

			pServerSocket->write(payload, [&writeCode](auto code) {
				writeCode = code;
			});
		});
		auto pClientSocket = test::AddClientReadBufferTask(pPool->ioContext(), receiveBuffer);
		pPool->join();

		// Assert:
		EXPECT_EQ(SocketOperationCode::Write_Error, writeCode);
		EXPECT_EQ(ByteBuffer(bufferSize), receiveBuffer);
	}

	TEST(TEST_CLASS, WriteFailsWhenClientSocketCloses) {
		// Arrange: set up payloads
		auto payload = CreateLargeWritePayload();
		SocketOperationCode writeCode;
		std::atomic<size_t> step(0);

		// Act: "server" - waits for the client socket to close; writes to the socket
		//      "client" - closes the client socket
		auto pPool = test::CreateStartedIoThreadPool();
		test::SpawnPacketServerWork(pPool->ioContext(), [&payload, &writeCode, &step](const auto& pServerSocket) {
			++step;
			WAIT_FOR_VALUE(2u, step);
			pServerSocket->write(payload, [&writeCode](auto code) {
				writeCode = code;
			});
		});
		test::CreateClientSocket(pPool->ioContext())->connect().then([&step](auto&& socketFuture) {
			WAIT_FOR_ONE(step);
			socketFuture.get()->shutdown();
			++step;
		});
		pPool->join();

		// Assert:
		test::AssertSocketClosedDuringWrite(writeCode);
	}

	TEST(TEST_CLASS, WriteCanWriteMultipleConsecutivePayloads) {
		test::AssertWriteCanWriteMultipleConsecutivePayloads([](const auto& pSocket) { return pSocket; });
	}

	TEST(TEST_CLASS, WriteFailsWhenPacketPayloadIsUnset) {
		// Arrange:
		auto payload = PacketPayload();

		// Assert:
		AssertWriteFailure(payload, 1'000);
	}

	TEST(TEST_CLASS, WriteFailsWhenPacketPayloadExceedsMaxPacketDataSize) {
		// Arrange:
		auto packetBytes = test::GenerateRandomPacketBuffer(150);
		auto payload = test::BufferToPacketPayload(packetBytes);

		// Assert:
		AssertWriteFailure(payload, 150 - sizeof(PacketHeader) - 1);
	}

	TEST(TEST_CLASS, WriteSucceedsWhenPacketPayloadIsExactlyMaxPacketDataSize) {
		// Arrange:
		auto packetBytes = test::GenerateRandomPacketBuffer(150);
		auto payload = test::BufferToPacketPayload(packetBytes);

		// Assert:
		AssertWriteSuccess(payload, packetBytes, 150 - sizeof(PacketHeader));
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
			result.IsPacketValid = !!pPacket;
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
			auto pPool = test::CreateStartedIoThreadPool();
			test::SpawnPacketServerWork(pPool->ioContext(), [&result](const auto& pServerSocket) {
				pServerSocket->read([pServerSocket, &result](auto code, const auto* pPacket) {
					FillResult(result, pServerSocket, code, pPacket);
				});
			});
			auto pClientSocket = test::AddClientWriteBuffersTask(pPool->ioContext(), sendBuffers);
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
		AssertSendBuffersResult(result, SocketOperationCode::Success, 18);
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
		AssertSendBuffersResult(result, SocketOperationCode::Success, 25);
		ASSERT_EQ(125u, receivedBuffer.size());
		EXPECT_EQ_MEMORY(&sendBuffers[0][0], &receivedBuffer[0], 50);
		EXPECT_EQ_MEMORY(&sendBuffers[1][0], &receivedBuffer[50], 50);
		EXPECT_EQ_MEMORY(&sendBuffers[2][0], &receivedBuffer[100], 25);
	}

	TEST(TEST_CLASS, ReadCanProcessFirstOfMultiplePacketsInSingleRead) {
		// Arrange: send a buffer containing three packets
		auto sendBuffer = test::GenerateRandomPacketBuffer(100, { 20, 17, 50 });
		std::vector<ByteBuffer> sendBuffers{ sendBuffer };

		// Act:
		auto result = SendBuffers(sendBuffers);
		const auto& receivedBuffer = result.ReceivedBuffer;

		// Assert: only the first buffer was returned
		AssertSendBuffersResult(result, SocketOperationCode::Success, 80);
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 0, 20u, receivedBuffer);
	}

	TEST(TEST_CLASS, ReadFailsOnReadError) {
		// Arrange: send one buffer containing one packet
		auto sendBuffer = test::GenerateRandomPacketBuffer(50, { 30 });
		std::vector<ByteBuffer> sendBuffers{ sendBuffer };

		// Act: "server" - closes the server socket and then attempts to read the next packet(s) from the socket
		//      "client" - sends all buffers to the socket
		SendBuffersResult result;
		auto pPool = test::CreateStartedIoThreadPool();
		test::SpawnPacketServerWork(pPool->ioContext(), [&result](const auto& pServerSocket) {
			pServerSocket->abort();
			CATAPULT_LOG(debug) << "closed server socket";

			pServerSocket->read([pServerSocket, &result](auto code, const auto* pPacket) {
				FillResult(result, pServerSocket, code, pPacket);
			});
		});
		auto pClientSocket = test::AddClientWriteBuffersTask(pPool->ioContext(), sendBuffers);
		pPool->join();

		// Assert: the server should get a read error when attempting to read from the socket
		//         (since nothing was read, the working buffer should be empty)
		AssertSendBuffersResult(result, SocketOperationCode::Read_Error, 0);
	}

	TEST(TEST_CLASS, ReadFailsWhenClientSocketCloses) {
		// Arrange:
		SocketOperationCode readCode;
		std::atomic<size_t> step(0);

		// Act: "server" - waits for the client socket to close; reads from the socket
		//      "client" - closes the client socket
		auto pPool = test::CreateStartedIoThreadPool();
		test::SpawnPacketServerWork(pPool->ioContext(), [&readCode, &step](const auto& pServerSocket) {
			++step;
			WAIT_FOR_VALUE(2u, step);
			pServerSocket->read([&readCode](auto code, const auto*) {
				readCode = code;
			});
		});
		test::CreateClientSocket(pPool->ioContext())->connect().then([&step](auto&& socketFuture) {
			WAIT_FOR_ONE(step);
			socketFuture.get()->shutdown();
			++step;
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
		AssertSendBuffersResult(result, SocketOperationCode::Malformed_Data, 100);
		EXPECT_TRUE(receivedBuffer.empty());
	}

	namespace {
		std::vector<SendBuffersResult> SendBuffersMultiple(const std::vector<ByteBuffer>& sendBuffers) {
			std::vector<SendBuffersResult> results;
			results.reserve(10);

			// Act: "server" - reads the next packet(s) from the socket (using readMultiple)
			//      "client" - sends all buffers to the socket
			auto pPool = test::CreateStartedIoThreadPool();
			test::SpawnPacketServerWork(pPool->ioContext(), [&results](const auto& pServerSocket) {
				pServerSocket->readMultiple([pServerSocket, &results](auto code, const auto* pPacket) {
					results.push_back(SendBuffersResult());
					FillResult(results.back(), pServerSocket, code, pPacket);
				});
			});
			auto pClientSocket = test::AddClientWriteBuffersTask(pPool->ioContext(), sendBuffers);
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
		AssertSendBuffersResult(results[0], SocketOperationCode::Success, 18);
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 0, 82u, receivedBuffer);

		AssertSendBuffersResult(results[1], SocketOperationCode::Insufficient_Data, 18);
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
		AssertSendBuffersResult(results[0], SocketOperationCode::Success, 25);
		ASSERT_EQ(125u, receivedBuffer.size());
		EXPECT_EQ_MEMORY(&sendBuffers[0][0], &receivedBuffer[0], 50);
		EXPECT_EQ_MEMORY(&sendBuffers[1][0], &receivedBuffer[50], 50);
		EXPECT_EQ_MEMORY(&sendBuffers[2][0], &receivedBuffer[100], 25);

		AssertSendBuffersResult(results[1], SocketOperationCode::Insufficient_Data, 25);
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
		AssertSendBuffersResult(results[0], SocketOperationCode::Success, 13);
		AssertSendBuffersResult(results[1], SocketOperationCode::Success, 13);
		AssertSendBuffersResult(results[2], SocketOperationCode::Success, 13);

		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 0, 20u, results[0].ReceivedBuffer);
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 20, 17u, results[1].ReceivedBuffer);
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 37, 50u, results[2].ReceivedBuffer);

		AssertSendBuffersResult(results[3], SocketOperationCode::Insufficient_Data, 13);
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
		ASSERT_EQ(2u, results.size());
		AssertSendBuffersResult(results[0], SocketOperationCode::Success, 80);
		AssertSendBuffersResult(results[1], SocketOperationCode::Malformed_Data, 80);

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
		auto pPool = test::CreateStartedIoThreadPool();
		test::SpawnPacketServerWork(pPool->ioContext(), [&](const auto& pServerSocket) {
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
		test::CreateClientSocket(pPool->ioContext())->connect()
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
		AssertSendBuffersResult(results2[0], SocketOperationCode::Success, 0);
		AssertSendBuffersResult(results2[1], SocketOperationCode::Insufficient_Data, 0);

		const auto& receivedBuffer1 = results1[0].ReceivedBuffer;
		ASSERT_EQ(Size_Unit * 5 / 2, receivedBuffer1.size());
		EXPECT_EQ_MEMORY(&sendBuffers[0][0], &receivedBuffer1[0], Size_Unit);
		EXPECT_EQ_MEMORY(&sendBuffers[1][0], &receivedBuffer1[Size_Unit], Size_Unit);
		EXPECT_EQ_MEMORY(&sendBuffers[2][0], &receivedBuffer1[2 * Size_Unit], Half_Size_Unit);

		const auto& receivedBuffer2 = results2[0].ReceivedBuffer;
		ASSERT_EQ(Size_Unit * 3 / 2, receivedBuffer2.size());

		EXPECT_EQ_MEMORY(&sendBuffers[2][Half_Size_Unit], &receivedBuffer2[0], Half_Size_Unit);
		EXPECT_EQ_MEMORY(&sendBuffers[3][0], &receivedBuffer2[Half_Size_Unit], Size_Unit);
	}

	TEST(TEST_CLASS, ReadCanReadMultipleConsecutivePayloads) {
		test::AssertReadCanReadMultipleConsecutivePayloads([](const auto& pSocket) { return pSocket; });
	}

	// endregion

	// region waitForData

	TEST(TEST_CLASS, WaitForDataIsNotTriggeredWhenNoDataIsPresent) {
		// Arrange:
		std::atomic<size_t> numCallbackCalls(0);

		// Act: "server" - waits for data to become available
		//      "client" - connects to the server
		auto pPool = test::CreateStartedIoThreadPool();
		test::SpawnPacketServerWork(pPool->ioContext(), [&numCallbackCalls](const auto& pServerSocket) {
			pServerSocket->waitForData([&numCallbackCalls]() {
				++numCallbackCalls;
			});
		});
		auto pClientSocket = test::AddClientConnectionTask(pPool->ioContext());
		pClientSocket.reset(); // server socket does not read the data
		pPool->join();

		// Assert:
		EXPECT_EQ(0u, numCallbackCalls);
	}

	TEST(TEST_CLASS, WaitForDataIsTriggeredWhenDataIsPresent) {
		// Arrange: send a single buffer containing a single packet
		std::atomic<size_t> numCallbackCalls(0);
		std::vector<ByteBuffer> sendBuffers{ test::GenerateRandomPacketBuffer(100) };

		// Act: "server" - waits for data to become available
		//      "client" - sends a single buffer
		auto pPool = test::CreateStartedIoThreadPool();
		test::SpawnPacketServerWork(pPool->ioContext(), [&numCallbackCalls](const auto& pServerSocket) {
			pServerSocket->waitForData([&numCallbackCalls]() {
				++numCallbackCalls;
			});
		});
		auto pClientSocket = test::AddClientWriteBuffersTask(pPool->ioContext(), sendBuffers);
		pClientSocket.reset(); // server socket does not read the data
		pPool->join();

		// Assert:
		EXPECT_EQ(1u, numCallbackCalls);
	}

	TEST(TEST_CLASS, ReadOrWaitForDataIsTriggered) {
		// Arrange: send a buffer containing a packet
		std::atomic<size_t> callbackMask(0);
		std::vector<ByteBuffer> sendBuffers{ test::GenerateRandomPacketBuffer(100) };

		// Act: "server" - in parallel try to async_read the data and wait for the data to become available
		//      "client" - sends a single buffer
		auto pPool = test::CreateStartedIoThreadPool();
		test::SpawnPacketServerWork(pPool->ioContext(), [&callbackMask](const auto& pServerSocket) {
			pServerSocket->waitForData([pServerSocket, &callbackMask]() {
				callbackMask += (1 << 8);
			});

			pServerSocket->read([pServerSocket, &callbackMask](auto, const auto*) {
				callbackMask += (1 << 4);
			});
		});
		auto pClientSocket = test::AddClientWriteBuffersTask(pPool->ioContext(), sendBuffers);
		pPool->join();

		// Assert:
		EXPECT_LE(0x0010u, callbackMask);
	}

	TEST(TEST_CLASS, WaitForDataDoesNotAffectRead) {
		// Arrange: send a single buffer containing a single packet
		SendBuffersResult result;
		std::atomic<size_t> numCallbackCalls(0);
		std::vector<ByteBuffer> sendBuffers{ test::GenerateRandomPacketBuffer(100) };

		// Act: "server" - waits for data to become available and reads the data
		//      "client" - sends a single buffer
		auto pPool = test::CreateStartedIoThreadPool();
		test::SpawnPacketServerWork(pPool->ioContext(), [&result, &numCallbackCalls](const auto& pServerSocket) {
			pServerSocket->waitForData([pServerSocket, &result, &numCallbackCalls]() {
				++numCallbackCalls;

				pServerSocket->read([pServerSocket, &result](auto code, const auto* pPacket) {
					FillResult(result, pServerSocket, code, pPacket);
				});
			});
		});
		auto pClientSocket = test::AddClientWriteBuffersTask(pPool->ioContext(), sendBuffers);
		pPool->join();

		// Assert:
		EXPECT_EQ(1u, numCallbackCalls);
		AssertSendBuffersResult(result, SocketOperationCode::Success, 0);
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 0, 100u, result.ReceivedBuffer);
	}

	// endregion

	// region close / abort

	TEST(TEST_CLASS, CloseTransitionsSocketToClosedState) {
		// Arrange:
		auto pPool = test::CreateStartedIoThreadPool();
		auto pAcceptor = test::CreateImplicitlyClosedLocalHostAcceptor(pPool->ioContext());

		// Act: "server" - accepts a connection and closes the socket
		//      "client" - connects to the server
		PacketSocket::Stats stats;
		std::shared_ptr<PacketSocket> pSocket;
		std::atomic_bool hasClosedSocket(false);
		boost::asio::post(pPool->ioContext(), [&ioContext = pPool->ioContext(), &acceptor = *pAcceptor, &pSocket, &hasClosedSocket]() {
			Accept(ioContext, acceptor, test::CreatePacketSocketOptions(), [&pSocket, &hasClosedSocket](const auto& acceptedSocketInfo) {
				pSocket = acceptedSocketInfo.socket();
				pSocket->close();
				hasClosedSocket = true;
			});
		});
		auto pClientSocket = test::AddClientConnectionTask(pPool->ioContext());

		// - close is async, so wait for it
		WAIT_FOR(hasClosedSocket);
		test::WaitForClosedSocket(*pSocket);
		pSocket->stats([&stats](const auto& socketStats) { stats = socketStats; });

		pPool->join();

		// Assert:
		EXPECT_TRUE(!!pSocket);
		EXPECT_FALSE(stats.IsOpen);
		EXPECT_EQ(0u, stats.NumUnprocessedBytes);
	}

	TEST(TEST_CLASS, AbortTransitionsSocketToClosedState) {
		// Arrange:
		auto pPool = test::CreateStartedIoThreadPool();
		auto pAcceptor = test::CreateImplicitlyClosedLocalHostAcceptor(pPool->ioContext());

		// Act: "server" - accepts a connection and closes the socket
		//      "client" - connects to the server
		PacketSocket::Stats stats;
		std::shared_ptr<PacketSocket> pSocket;
		boost::asio::post(pPool->ioContext(), [&ioContext = pPool->ioContext(), &acceptor = *pAcceptor, &stats, &pSocket]() {
			Accept(ioContext, acceptor, test::CreatePacketSocketOptions(), [&stats, &pSocket](const auto& acceptedSocketInfo) {
				pSocket = acceptedSocketInfo.socket();
				pSocket->abort();

				// - abort is sync, so no need to wait
				pSocket->stats([&stats](const auto& socketStats) { stats = socketStats; });
			});
		});
		auto pClientSocket = test::AddClientConnectionTask(pPool->ioContext());
		pPool->join();

		// Assert:
		EXPECT_TRUE(!!pSocket);
		EXPECT_FALSE(stats.IsOpen);
		EXPECT_EQ(0u, stats.NumUnprocessedBytes);
	}

	TEST(TEST_CLASS, CanDestroyWithPendingOperations) {
		// Arrange: send one buffer containing one packet
		auto sendBuffer = test::GenerateRandomPacketBuffer(50, { 30 });
		std::vector<ByteBuffer> sendBuffers{ sendBuffer };

		// Act: "server" - start a read and destroy the server socket
		//      "client" - sends all buffers to the socket
		SendBuffersResult result;
		auto pPool = test::CreateStartedIoThreadPool();
		test::SpawnPacketServerWork(pPool->ioContext(), [&result](const auto& pServerSocket) {
			pServerSocket->read([&result](auto code, const auto* pPacket) {
				result.Code = code;
				result.IsPacketValid = !!pPacket;
				if (result.IsPacketValid)
					result.ReceivedBuffer = test::CopyPacketToBuffer(*pPacket);
			});

			std::const_pointer_cast<PacketSocket>(pServerSocket).reset();
			CATAPULT_LOG(debug) << "destroyed server socket";
		});
		auto pClientSocket = test::AddClientWriteBuffersTask(pPool->ioContext(), sendBuffers);
		pPool->join();

		// Assert: the read should have completed successfully
		EXPECT_EQ(SocketOperationCode::Success, result.Code);
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 0, 30u, result.ReceivedBuffer);
	}

	// endregion

	// region synchronization

	TEST(TEST_CLASS, OperationsOnSingleSocketAreStranded) {
		// Arrange:
		auto pPool = test::CreateStartedIoThreadPool();
		auto pAcceptor = test::CreateImplicitlyClosedLocalHostAcceptor(pPool->ioContext());

		// Act: "server" - accepts a connection and posts a few operations on the socket
		//      "client" - connects to the server
		std::shared_ptr<PacketSocket> pSocket;
		std::vector<std::string> breadcrumbs;
		boost::asio::post(pPool->ioContext(), [&ioContext = pPool->ioContext(), &acceptor = *pAcceptor, &breadcrumbs, &pSocket]() {
			Accept(ioContext, acceptor, test::CreatePacketSocketOptions(), [&breadcrumbs, &pSocket](const auto& acceptedSocketInfo) {
				pSocket = acceptedSocketInfo.socket();
				auto addStatsBreadcrumb = [&breadcrumbs](const auto& stats) {
					breadcrumbs.push_back("open? " + std::to_string(stats.IsOpen));
				};

				for (auto i = 0u; i < 4; ++i)
					pSocket->stats(addStatsBreadcrumb);

				pSocket->abort();

				for (auto i = 0u; i < 4; ++i)
					pSocket->stats(addStatsBreadcrumb);
			});
		});
		auto pClientSocket = test::AddClientConnectionTask(pPool->ioContext());
		pPool->join();

		// Assert: if the operations were not synchronized, the ordering would not be guaranteed (4 open, 4 closed)
		std::vector<std::string> expectedBreadcrumbs{
			"open? 1", "open? 1", "open? 1", "open? 1",
			"open? 0", "open? 0", "open? 0", "open? 0"
		};
		EXPECT_EQ(expectedBreadcrumbs, breadcrumbs);
	}

	// endregion

	// region PacketSocketInfo

	TEST(TEST_CLASS, PacketSocketInfo_CanCreateDefault) {
		// Act:
		PacketSocketInfo socketInfo;

		// Assert:
		EXPECT_EQ("", socketInfo.host());
		EXPECT_EQ(Key(), socketInfo.publicKey());
		EXPECT_FALSE(!!socketInfo.socket());
	}

	TEST(TEST_CLASS, PacketSocketInfo_CanCreateAroundSocketAndIdentity) {
		// Arrange:
		auto publicKey = test::GenerateRandomByteArray<Key>();
		auto pSocket = std::make_shared<mocks::MockPacketSocket>();

		// Act:
		PacketSocketInfo socketInfo("a.b.c", publicKey, pSocket);

		// Assert:
		EXPECT_EQ("a.b.c", socketInfo.host());
		EXPECT_EQ(publicKey, socketInfo.publicKey());
		EXPECT_EQ(pSocket, socketInfo.socket());
	}

	// endregion

	// region Accept

	TEST(TEST_CLASS, AcceptReturnsOpenSocketOnSuccess) {
		// Arrange:
		auto publicKey = test::GenerateRandomByteArray<Key>();
		auto pPool = test::CreateStartedIoThreadPool();
		auto& ioContext = pPool->ioContext();
		auto pAcceptor = test::CreateImplicitlyClosedLocalHostAcceptor(ioContext);

		// Act: "server" - accepts a connection
		//      "client" - connects to the server
		PacketSocket::Stats stats;
		PacketSocketInfo socketInfo;
		boost::asio::post(ioContext, [&ioContext, &acceptor = *pAcceptor, publicKey, &stats, &socketInfo]() {
			Accept(ioContext, acceptor, test::CreatePacketSocketOptions(publicKey), [&stats, &socketInfo](const auto& acceptedSocketInfo) {
				socketInfo = acceptedSocketInfo;
				socketInfo.socket()->stats([&stats](const auto& socketStats) { stats = socketStats; });
			});
		});
		auto pClientSocket = test::AddClientConnectionTask(ioContext);
		pPool->join();

		// Assert: loopback address is used in tests
		EXPECT_TRUE(!!socketInfo);
		EXPECT_EQ("127.0.0.1", socketInfo.host());
		EXPECT_EQ(publicKey, socketInfo.publicKey());
		EXPECT_TRUE(!!socketInfo.socket());

		EXPECT_TRUE(stats.IsOpen);
		EXPECT_EQ(0u, stats.NumUnprocessedBytes);
	}

	TEST(TEST_CLASS, AcceptReturnsNullSocketOnAcceptFailure) {
		// Arrange: notice acceptor is not listening
		auto pPool = test::CreateStartedIoThreadPool();
		auto& ioContext = pPool->ioContext();
		auto pAcceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(ioContext);

		// Act: "server" - does not listen for connections
		//      "client" - connects to the server
		PacketSocketInfo socketInfo;
		std::atomic_bool isCallbackCalled(false);
		boost::asio::post(ioContext, [&ioContext, &acceptor = *pAcceptor, &socketInfo, &isCallbackCalled]() {
			auto options = test::CreatePacketSocketOptions();
			Accept(ioContext, acceptor, options, [&socketInfo, &isCallbackCalled](const auto& acceptedSocketInfo) {
				socketInfo = acceptedSocketInfo;
				isCallbackCalled = true;
			});
		});
		auto pClientSocket = test::AddClientConnectionTask(ioContext);
		pPool->join();

		// Assert:
		test::AssertEmpty(socketInfo);

		EXPECT_TRUE(isCallbackCalled);
	}

	TEST(TEST_CLASS, AcceptReturnsNullSocketOnHandshakeFailure) {
		// Arrange:
		auto pPool = test::CreateStartedIoThreadPool();
		auto& ioContext = pPool->ioContext();
		auto pAcceptor = test::CreateImplicitlyClosedLocalHostAcceptor(ioContext);

		// Act: "server" - accepts a connection
		//      "client" - connects to the server but does not participate in handshake and disconnects
		PacketSocketInfo socketInfo;
		std::atomic_bool isCallbackCalled(false);
		boost::asio::post(ioContext, [&ioContext, &acceptor = *pAcceptor, &socketInfo, &isCallbackCalled]() {
			auto options = test::CreatePacketSocketOptions();
			Accept(ioContext, acceptor, options, [&socketInfo, &isCallbackCalled](const auto& acceptedSocketInfo) {
				socketInfo = acceptedSocketInfo;
				isCallbackCalled = true;
			});
		});
		test::CreateClientSocket(ioContext)->connect(test::ClientSocket::ConnectOptions::Skip_Handshake);
		pPool->join();

		// Assert:
		test::AssertEmpty(socketInfo);

		EXPECT_TRUE(isCallbackCalled);
	}

	TEST(TEST_CLASS, AcceptReturnsNullSocketWhenHandshakeExceedsTimeout) {
		// Arrange:
		auto pPool = test::CreateStartedIoThreadPool();
		auto& ioContext = pPool->ioContext();
		auto pAcceptor = test::CreateImplicitlyClosedLocalHostAcceptor(ioContext);

		// Act: "server" - accepts a connection
		//      "client" - connects to the server but does not participate in handshake and stays connected
		PacketSocketInfo socketInfo;
		std::atomic_bool isCallbackCalled(false);
		boost::asio::post(ioContext, [&ioContext, &acceptor = *pAcceptor, &socketInfo, &isCallbackCalled]() {
			auto options = test::CreatePacketSocketOptions();
			options.AcceptHandshakeTimeout = utils::TimeSpan::FromSeconds(1);
			Accept(ioContext, acceptor, options, [&socketInfo, &isCallbackCalled](const auto& acceptedSocketInfo) {
				socketInfo = acceptedSocketInfo;
				isCallbackCalled = true;
			});
		});

		auto pClientSocket = test::CreateClientSocket(ioContext);
		pClientSocket->connect(test::ClientSocket::ConnectOptions::Skip_Handshake);
		pPool->join();

		// Assert:
		test::AssertEmpty(socketInfo);

		EXPECT_TRUE(isCallbackCalled);
	}

// disable for windows because getpeername appears to succeed even after socket is closed
#ifndef _WIN32

	TEST(TEST_CLASS, AcceptReturnsNullSocketOnRemoteEndpointFailure) {
		// Arrange: non-deterministic because timing dependent on server and client behavior
		//          this is a race condition that is difficult to reproduce, requiring:
		//          1. delay between async_accept success and remote_endpoint query
		//          2. client that sets SO_LINGER to 0s and immediately closes connection within (1) delay period
		test::RunNonDeterministicTest("remote endpoint failure", 50 * test::GetMaxNonDeterministicTestRetries(), [](auto) {
			auto pPool = test::CreateStartedIoThreadPool(1);
			auto& ioContext = pPool->ioContext();
			auto pAcceptor = test::CreateImplicitlyClosedLocalHostAcceptor(ioContext);

			// Act: "server" - accepts a connection
			//      "client" - connects to the server and immediately aborts the connection
			PacketSocketInfo socketInfo;
			std::atomic_bool isCallbackCalled(false);
			boost::asio::post(pPool->ioContext(), [&ioContext, &acceptor = *pAcceptor, &socketInfo, &isCallbackCalled]() {
				auto options = test::CreatePacketSocketOptions();
				Accept(ioContext, acceptor, options, [&socketInfo, &isCallbackCalled](const auto& acceptedSocketInfo) {
					socketInfo = acceptedSocketInfo;
					isCallbackCalled = true;
				});
			});
			test::CreateClientSocket(pPool->ioContext())->connect(test::ClientSocket::ConnectOptions::Abort);
			pPool->join();

			if (socketInfo)
				return false;

			// Assert:
			test::AssertEmpty(socketInfo);

			EXPECT_TRUE(isCallbackCalled);
			return true;
		});
	}

#endif

	namespace {
		template<typename TAction>
		void RunAcceptHonorsOptionsTest(uint32_t bufferSize, uint32_t adjustmentSize, TAction action) {
			// Arrange:
			auto sendBuffer = test::GenerateRandomPacketBuffer(bufferSize);
			auto pPool = test::CreateStartedIoThreadPool();
			auto pAcceptor = test::CreateImplicitlyClosedLocalHostAcceptor(pPool->ioContext());

			// Act: "server" - accepts a connection and reads a buffer
			//      "client" - connects to the server and writes a buffer
			SendBuffersResult result;
			auto options = test::CreatePacketSocketOptions();
			options.MaxPacketDataSize = bufferSize - sizeof(PacketHeader) - adjustmentSize;
			boost::asio::post(pPool->ioContext(), [&ioContext = pPool->ioContext(), &acceptor = *pAcceptor, &options, &result]() {
				Accept(ioContext, acceptor, options, [&result](const auto& acceptedSocketInfo) {
					auto pServerSocket = acceptedSocketInfo.socket();
					pServerSocket->read([pServerSocket, &result](auto code, const auto* pPacket) {
						FillResult(result, pServerSocket, code, pPacket);
					});
				});
			});
			auto pClientSocket = test::AddClientWriteBuffersTask(pPool->ioContext(), { sendBuffer });
			pPool->join();

			// Assert:
			action(result);
		}
	}

	TEST(TEST_CLASS, AcceptHonorsOptions_MaxPacketDataSize) {
		// Act:
		RunAcceptHonorsOptionsTest(150, 0, [](const auto& result) {
			// Assert: the packet should be allowed (this test is just checking a limit, so doesn't need to check buffers)
			AssertSendBuffersResult(result, SocketOperationCode::Success, 0);
		});
	}

	TEST(TEST_CLASS, AcceptHonorsOptions_MaxPacketDataSizeExceeded) {
		// Act:
		RunAcceptHonorsOptionsTest(150, 1, [](const auto& result) {
			// Assert: the packet should be flagged as malformed because it is too large
			AssertSendBuffersResult(result, SocketOperationCode::Malformed_Data, 150);
		});
	}

	// endregion

	// region Connect

	namespace {
		struct ConnectResultTuple {
			ConnectResult Result;
			std::string Host;
			Key PublicKey;
			bool IsSocketValid;
		};

		auto Connect(
				boost::asio::io_context& ioContext,
				const PacketSocketOptions& options,
				const NodeEndpoint& endpoint,
				ConnectResultTuple& result) {
			return ionet::Connect(ioContext, options, endpoint, [&result](auto connectResult, const auto& connectedSocketInfo) {
				result.Result = connectResult;
				result.Host = connectedSocketInfo.host();
				result.PublicKey = connectedSocketInfo.publicKey();
				result.IsSocketValid = !!connectedSocketInfo;
			});
		}

		ConnectResultTuple ConnectAndWait(
				boost::asio::io_context& ioContext,
				const PacketSocketOptions& options,
				const NodeEndpoint& endpoint) {
			// Act:
			ConnectResultTuple result;
			Connect(ioContext, options, endpoint, result);

			// - wait for all work to complete
			ioContext.run();
			return result;
		}

		ConnectResultTuple ConnectAndWait(boost::asio::io_context& ioContext, const NodeEndpoint& endpoint, const Key& publicKey) {
			auto options = test::CreatePacketSocketOptions(publicKey);
			return ConnectAndWait(ioContext, options, endpoint);
		}

		ConnectResultTuple ConnectAndWait(const NodeEndpoint& endpoint) {
			boost::asio::io_context ioContext;
			auto publicKey = test::GenerateRandomByteArray<Key>();
			return ConnectAndWait(ioContext, endpoint, publicKey);
		}
	}

	TEST(TEST_CLASS, ConnectionFailsWhenNodeEndpointCannotBeResolved) {
		// Act: attempt to connect to an invalid hostname
		auto result = ConnectAndWait({ "127.0.0.X", test::GetLocalHostPort() });

		// Assert:
		EXPECT_EQ(ConnectResult::Resolve_Error, result.Result);
		EXPECT_FALSE(result.IsSocketValid);
	}

	TEST(TEST_CLASS, ConnectionFailsWhenServerRefusesConnection) {
		// Act: attempt to connect to a local server that hasn't been started
		auto result = ConnectAndWait(test::CreateLocalHostNodeEndpoint());

		// Assert:
		EXPECT_EQ(ConnectResult::Connect_Error, result.Result);
		EXPECT_FALSE(result.IsSocketValid);
	}

	TEST(TEST_CLASS, ConnectionFailsWhenServerHandshakeFails) {
		// Arrange: set up a server acceptor thread
		auto publicKey = test::GenerateRandomByteArray<Key>();
		boost::asio::io_context ioContext;
		auto pAcceptor = test::CreateImplicitlyClosedLocalHostAcceptor(ioContext);

		// - "server" - accepts a connection but does not initiate a handshake
		boost::asio::post(ioContext, [&acceptor = *pAcceptor]() {
			acceptor.async_accept([](const auto& acceptEc, const auto&) {
				CATAPULT_LOG(debug) << "async_accept completed with: " << acceptEc.message();
			});
		});

		// Act: attempt to connect to a running server
		auto result = ConnectAndWait(ioContext, test::CreateLocalHostNodeEndpoint(), publicKey);

		// Assert:
		EXPECT_EQ(ConnectResult::Handshake_Error, result.Result);
		EXPECT_FALSE(result.IsSocketValid);
	}

	TEST(TEST_CLASS, ConnectionSucceedsWhenServerAcceptsConnection) {
		// Arrange: set up a server acceptor thread (this is required to perform ssl handshake)
		auto publicKey = test::GenerateRandomByteArray<Key>();
		boost::asio::io_context ioContext;
		test::SpawnPacketServerWork(ioContext, [](const auto&) {});

		// Act: attempt to connect to a running server
		auto result = ConnectAndWait(ioContext, test::CreateLocalHostNodeEndpoint(), publicKey);

		// Assert:
		EXPECT_EQ(ConnectResult::Connected, result.Result);
		EXPECT_EQ("127.0.0.1", result.Host);
		EXPECT_EQ(publicKey, result.PublicKey);
		EXPECT_TRUE(result.IsSocketValid);
	}

	namespace {
		bool RunCancellationTestIteration() {
			// Arrange: set up a server thread
			auto pPool = test::CreateStartedIoThreadPool();
			auto& ioContext = pPool->ioContext();

			CATAPULT_LOG(debug) << "starting async accept";
			auto pAcceptor = test::CreateImplicitlyClosedLocalHostAcceptor(ioContext);
			auto acceptorStrand = boost::asio::io_context::strand(ioContext);
			auto serverSocket = boost::asio::ip::tcp::socket(ioContext);

			// - post an async server accept
			boost::asio::post(acceptorStrand, [&acceptor = *pAcceptor, &serverSocket]() {
				acceptor.async_accept(serverSocket, [](const auto& acceptEc) {
					CATAPULT_LOG(debug) << "async_accept completed with: " << acceptEc.message();
				});
			});

			// Act: attempt to connect to a running local server
			ConnectResultTuple result;
			auto cancel = Connect(ioContext, test::CreatePacketSocketOptions(), test::CreateLocalHostNodeEndpoint(), result);

			// - immediately cancel the connect
			cancel();

			// - cancel any pending server accepts (depending on timing, either the resolve or connect could be cancelled)
			boost::asio::post(acceptorStrand, [&acceptor = *pAcceptor]() {
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

	TEST(TEST_CLASS, ConnectionFailsWhenConnectAttemptIsCancelled) {
		// Assert: non-deterministic because a socket could connect before it is cancelled
		test::RunNonDeterministicTest("Cancellation", RunCancellationTestIteration);
	}

	namespace {
		template<typename TAction>
		void RunConnectHonorsOptionsTest(uint32_t bufferSize, uint32_t adjustmentSize, TAction action) {
			// Arrange:
			auto sendBuffer = test::GenerateRandomPacketBuffer(bufferSize);
			auto pPool = test::CreateStartedIoThreadPool();

			// Act: "server" - accepts a connection and writes a buffer
			//      "client" - connects to the server and reads a buffer
			SendBuffersResult result;
			test::SpawnPacketServerWork(pPool->ioContext(), [&sendBuffer](const auto& pServerSocket) {
				pServerSocket->write(test::BufferToPacketPayload(sendBuffer), [](auto code) {
					CATAPULT_LOG(debug) << "write completed with code " << code;
				});
			});

			auto endpoint = test::CreateLocalHostNodeEndpoint();
			auto options = test::CreatePacketSocketOptions();
			options.MaxPacketDataSize = bufferSize - sizeof(PacketHeader) - adjustmentSize;
			ionet::Connect(pPool->ioContext(), options, endpoint, [&result](auto, const auto& connectedSocketInfo) {
				auto pClientSocket = connectedSocketInfo.socket();
				pClientSocket->read([pClientSocket, &result](auto code, const auto* pPacket) {
					CATAPULT_LOG(debug) << "read completed with code " << code;
					FillResult(result, pClientSocket, code, pPacket);
				});
			});
			pPool->join();

			// Assert:
			action(result);
		}
	}

	TEST(TEST_CLASS, ConnectHonorsOptions_MaxPacketDataSize) {
		// Act:
		RunConnectHonorsOptionsTest(150, 0, [](const auto& result) {
			// Assert: the packet should be allowed (this test is just checking a limit, so doesn't need to check buffers)
			AssertSendBuffersResult(result, SocketOperationCode::Success, 0);
		});
	}

	TEST(TEST_CLASS, ConnectHonorsOptions_MaxPacketDataSizeExceeded) {
		// Act:
		RunConnectHonorsOptionsTest(150, 1, [](const auto& result) {
			// Assert: the packet should be flagged as malformed because it is too large
			//         do not use AssertSendBuffersResult beacause the number of unprocessed bytes is non-deterministic
			//         either the packet header or the entire packet will have been read (the header data marks the packet as malformed)
			EXPECT_EQ(SocketOperationCode::Malformed_Data, result.Code);
			EXPECT_LE(sizeof(PacketHeader), result.NumUnprocessedBytes);
			EXPECT_EQ(1u, result.NumHandlerCalls);
			EXPECT_FALSE(result.IsPacketValid);
		});
	}

	// endregion

	// region Connect - IPv4 / IPv6

	namespace {
		auto CreateEndpointForListenInterface(const std::string& listenInterface) {
			return boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(listenInterface), test::GetLocalHostPort());
		}

		void RunConnectionAcceptedTest(const std::string& listenInterface, const std::string& remoteHost, IpProtocol outgoingProtocols) {
			// Arrange: set up a server acceptor thread (this is required to perform ssl handshake)
			auto publicKey = test::GenerateRandomByteArray<Key>();

			boost::asio::io_context ioContext;
			auto pAcceptor = std::make_shared<test::TcpAcceptor>(ioContext, CreateEndpointForListenInterface(listenInterface));
			test::SpawnPacketServerWork(*pAcceptor, [pAcceptor](const auto&) {});
			pAcceptor.reset();

			// Act: attempt to connect to a running server
			auto options = test::CreatePacketSocketOptions(publicKey);
			options.OutgoingProtocols = outgoingProtocols;

			auto endpoint = NodeEndpoint{ remoteHost, test::GetLocalHostPort() };
			auto result = ConnectAndWait(ioContext, options, endpoint);

			// Assert:
			EXPECT_EQ(ConnectResult::Connected, result.Result);
			EXPECT_EQ(remoteHost, result.Host);
			EXPECT_EQ(publicKey, result.PublicKey);
			EXPECT_TRUE(result.IsSocketValid);
		}

		void RunConnectionRejectedTest(const std::string& remoteHost, IpProtocol outgoingProtocols) {
			// Arrange: do not set up a server acceptor thread (failure is after resolve but before connect)
			auto publicKey = test::GenerateRandomByteArray<Key>();

			boost::asio::io_context ioContext;

			// Act: attempt to connect to a stopped server
			auto options = test::CreatePacketSocketOptions(publicKey);
			options.OutgoingProtocols = outgoingProtocols;

			auto endpoint = NodeEndpoint{ remoteHost, test::GetLocalHostPort() };
			auto result = ConnectAndWait(ioContext, options, endpoint);

			// Assert:
			EXPECT_EQ(ConnectResult::Resolve_Error, result.Result);
			EXPECT_FALSE(result.IsSocketValid);
		}
	}

	TEST(TEST_CLASS, ConnectorWithIpv4Mask_CanConnectViaIpv4) {
		RunConnectionAcceptedTest("0.0.0.0", "127.0.0.1", IpProtocol::IPv4);
	}

	TEST(TEST_CLASS, ConnectorWithIpv4Mask_CannotConnectViaIpv6) {
		RunConnectionRejectedTest("::1", IpProtocol::IPv4);
	}

	TEST(TEST_CLASS, ConnectorWithIpv6Mask_CannotConnectViaIpv4) {
		RunConnectionRejectedTest("127.0.0.1", IpProtocol::IPv6);
	}

	TEST(TEST_CLASS, ConnectorWithIpv6Mask_CanConnectViaIpv6) {
		RunConnectionAcceptedTest("::", "::1", IpProtocol::IPv6);
	}

	TEST(TEST_CLASS, ConnectorWithIpv4Ipv6Mask_CanConnectViaIpv4) {
		RunConnectionAcceptedTest("0.0.0.0", "127.0.0.1", IpProtocol::IPv4 | IpProtocol::IPv6);
	}

	TEST(TEST_CLASS, ConnectorWithIpv4Ipv6Mask_CanConnectViaIpv6) {
		RunConnectionAcceptedTest("::", "::1", IpProtocol::IPv4 | IpProtocol::IPv6);
	}

	// endregion
}}
