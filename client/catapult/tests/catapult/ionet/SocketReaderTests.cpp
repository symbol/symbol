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

#include "catapult/ionet/SocketReader.h"
#include "catapult/ionet/BufferedPacketIo.h"
#include "catapult/ionet/IoTypes.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/thread/IoThreadPool.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/net/ClientSocket.h"
#include "tests/test/net/SocketTestUtils.h"
#include "tests/test/nodeps/Atomics.h"
#include "tests/TestHarness.h"
#include <set>

namespace catapult { namespace ionet {

#define TEST_CLASS SocketReaderTests

	namespace {
		struct SocketReadResult {
			std::vector<SocketOperationCode> CompletionCodes;
			size_t NumUnconsumedBytes;
		};

		class ReaderFactory {
		public:
			ReaderFactory() : ReaderFactory(test::GenerateRandomByteArray<Key>(), std::string())
			{}

			ReaderFactory(const Key& clientPublicKey, const std::string& clientHost)
					: m_pPool(test::CreateStartedIoThreadPool())
					, m_clientPublicKey(clientPublicKey)
					, m_clientHost(clientHost)
			{}

		public:
			std::unique_ptr<SocketReader> createReader(
					const std::shared_ptr<PacketSocket>& pSocket,
					const ServerPacketHandlers& handlers) {
				auto pBufferedIo = pSocket->buffered();
				return ionet::CreateSocketReader(pSocket, pBufferedIo, handlers, { m_clientPublicKey, m_clientHost });
			}

			template<typename TContinuation>
			void createReader(const ServerPacketHandlers& handlers, TContinuation continuation) {
				test::SpawnPacketServerWork(m_pPool->ioContext(), [this, &handlers, continuation](const auto& pServerSocket) {
					continuation(pServerSocket, this->createReader(pServerSocket, handlers));
				});
			}

			// starts a reader around \a handlers to update \a readResult
			template<typename TContinuation>
			void startReader(const ServerPacketHandlers& handlers, SocketReadResult& readResult, TContinuation continuation) {
				createReader(handlers, [this, &readResult, continuation](const auto& pSocket, auto&& pReader) {
					this->startReader(*pReader, pSocket, readResult);
					continuation(std::move(pReader));
				});
			}

			// starts a reader on \a pSocket, around \a handlers to update \a readResult
			template<typename TContinuation>
			void startReader(
					const std::shared_ptr<PacketSocket>& pSocket,
					const ServerPacketHandlers& handlers,
					SocketReadResult& readResult,
					TContinuation continuation) {
				auto pReader = createReader(pSocket, handlers);
				startReader(*pReader, pSocket, readResult);
				continuation(std::move(pReader));
			}

		private:
			void startReader(SocketReader& reader, const std::shared_ptr<PacketSocket>& pSocket, SocketReadResult& readResult) {
				reader.read([pSocket, &readResult](auto result) {
					readResult.CompletionCodes.push_back(result);
					pSocket->stats([&readResult](const auto& stats) {
						readResult.NumUnconsumedBytes = stats.NumUnprocessedBytes;
					});
				});
			}

		public:
			auto& ioContext() {
				return m_pPool->ioContext();
			}

			void join() {
				m_pPool->join();
			}

		private:
			std::unique_ptr<thread::IoThreadPool> m_pPool;
			Key m_clientPublicKey;
			std::string m_clientHost;
		};

		// creates server packet handlers that have a noop registered for the default packet type
		ServerPacketHandlers CreateNoOpHandlers() {
			ServerPacketHandlers handlers;
			handlers.registerHandler(test::Default_Packet_Type, [](const auto&, const auto&) {});
			return handlers;
		}

		// writes all \a sendBuffers to a socket and reads them with a reader
		std::pair<std::vector<ByteBuffer>, SocketReadResult> SendBuffers(const std::vector<ByteBuffer>& sendBuffers) {
			// Arrange: set up a packet handler that copies the received packet bytes into receivedBuffers
			ReaderFactory factory;
			ServerPacketHandlers handlers;
			std::vector<ByteBuffer> receivedBuffers;
			test::AddCopyBuffersHandler(handlers, receivedBuffers);

			// Act: "server" - reads packets from the socket using the reader
			//      "client" - writes sendBuffers to the socket
			SocketReadResult readResult;
			std::unique_ptr<SocketReader> pReader;
			factory.startReader(handlers, readResult, [&pReader](auto&& pStartedReader) {
				pReader = std::move(pStartedReader);
			});
			auto pClientSocket = test::AddClientWriteBuffersTask(factory.ioContext(), sendBuffers);

			// - wait for the test to complete
			factory.join();
			return std::make_pair(receivedBuffers, readResult);
		}

		// sets a response packet in \a handlerContext with payload \a responseBytes
		void RespondWithBytes(ServerPacketHandlerContext& handlerContext, const std::vector<uint8_t>& responseBytes) {
			auto numResponseBytes = static_cast<uint32_t>(responseBytes.size());
			auto pResponsePacket = CreateSharedPacket<Packet>(numResponseBytes);
			pResponsePacket->Size = SizeOf32<PacketHeader>() + numResponseBytes;
			utils::memcpy_cond(static_cast<void*>(pResponsePacket.get() + 1), responseBytes.data(), responseBytes.size());
			handlerContext.response(PacketPayload(pResponsePacket));
		}

		void AssertSocketReadResult(
				const SocketReadResult& result,
				const std::vector<SocketOperationCode>& expectedCodes,
				size_t expectedNumUnconsumedBytes) {
			// Assert:
			EXPECT_EQ(expectedNumUnconsumedBytes, result.NumUnconsumedBytes);
			EXPECT_EQ(expectedCodes, result.CompletionCodes);
		}

		void AssertSocketReadSuccess(const SocketReadResult& result, size_t expectedNumReads, size_t expectedNumUnconsumedBytes) {
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

	TEST(TEST_CLASS, CanReadSinglePacket) {
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

	TEST(TEST_CLASS, CanReadSinglePacketSpanningReads) {
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
		EXPECT_EQ_MEMORY(&sendBuffers[0][0], &receivedBuffers[0][0], 50);
		EXPECT_EQ_MEMORY(&sendBuffers[1][0], &receivedBuffers[0][50], 50);
		EXPECT_EQ_MEMORY(&sendBuffers[2][0], &receivedBuffers[0][100], 25);
	}

	TEST(TEST_CLASS, CanReadMultiplePacketsInSingleRead) {
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

	TEST(TEST_CLASS, CanRespondToPacket) {
		// Arrange: send a buffer containing three packets
		auto sendBuffer = test::GenerateRandomPacketBuffer(100, { 0x14, 0x11, 0x32, 0x19 });
		std::vector<ByteBuffer> sendBuffers{ sendBuffer };

		// - set up a packet handler that sends the sizes of all previously received packets back to the client
		//   as well as the total number of received packets
		ReaderFactory factory;
		ServerPacketHandlers handlers;
		std::vector<uint32_t> packetSizes;
		test::RegisterDefaultHandler(handlers, [&packetSizes](const auto& packet, auto& context) {
			auto packetSize = static_cast<uint32_t>(packet.Size); // Size may be misaligned and cannot be bound to reference
			packetSizes.push_back(packetSize);

			std::vector<uint8_t> responseBytes{ static_cast<uint8_t>(packetSizes.size()) };
			for (auto size : packetSizes)
				responseBytes.push_back(static_cast<uint8_t>(size));

			RespondWithBytes(context, responseBytes);
		});

		// Act: "server" - reads packets from the socket using the reader
		//      "client" - writes sendBuffers to the socket and reads the response data into responseBytes
		SocketReadResult readResult;
		std::vector<uint8_t> responseBytes(3 * sizeof(Packet) + 9);
		std::unique_ptr<SocketReader> pReader;
		factory.startReader(handlers, readResult, [&pReader](auto&& pStartedReader) {
			pReader = std::move(pStartedReader);
		});
		test::CreateClientSocket(factory.ioContext())->connect().then([&sendBuffers, &responseBytes](auto&& socketFuture) {
			auto* pClientSocket = socketFuture.get();
			pClientSocket->write(sendBuffers).then([pClientSocket, &responseBytes](auto) {
				pClientSocket->read(responseBytes);
			});
		});

		// - wait for the test to complete
		factory.join();

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
		EXPECT_EQ(expectedClientBytes, responseBytes);
	}

	TEST(TEST_CLASS, ReadFailsOnReadError) {
		// Arrange: send one buffer containing one packet
		auto sendBuffer = test::GenerateRandomPacketBuffer(50, { 30, 70 });
		std::vector<ByteBuffer> sendBuffers{ sendBuffer };
		ReaderFactory factory;
		auto handlers = CreateNoOpHandlers();

		// Act: "server" - closes the socket and then reads packets from the socket using the reader
		//      "client" - writes sendBuffers to the socket
		SocketReadResult readResult;
		std::unique_ptr<SocketReader> pReader;
		test::SpawnPacketServerWork(factory.ioContext(), [&](const auto& pServerSocket) {
			pServerSocket->abort();
			factory.startReader(pServerSocket, handlers, readResult, [&pReader](auto&& pStartedReader) {
				pReader = std::move(pStartedReader);
			});
		});
		auto pClientSocket = test::AddClientWriteBuffersTask(factory.ioContext(), sendBuffers);

		// - wait for the test to complete
		factory.join();

		// Assert: the server should get a read error when attempting to read the buffer from the client
		//         (since the read failed, no bytes were read and none were consumed)
		AssertSocketReadFailure(readResult, 1, SocketOperationCode::Read_Error, 0);
	}

	TEST(TEST_CLASS, ReadFailsForUnknownPacket) {
		// Arrange: send one buffer containing one packet
		auto sendBuffer = test::GenerateRandomPacketBuffer(50, { 30, 70 });
		std::vector<ByteBuffer> sendBuffers{ sendBuffer };
		ReaderFactory factory;
		ServerPacketHandlers handlers;

		// Act: "server" - reads packets from the socket using the reader but does not have any handlers registered
		//      "client" - writes sendBuffers to the socket
		SocketReadResult readResult;
		std::unique_ptr<SocketReader> pReader;
		factory.startReader(handlers, readResult, [&pReader](auto&& pStartedReader) {
			pReader = std::move(pStartedReader);
		});
		auto pClientSocket = test::AddClientWriteBuffersTask(factory.ioContext(), sendBuffers);

		// - wait for the test to complete
		factory.join();

		// Assert: the server treats the unknown packet as malformed
		//         (the 20 unconsumed bytes are the remainder in the buffer after consuming the first packet)
		AssertSocketReadFailure(readResult, 1, SocketOperationCode::Malformed_Data, 20);
	}

	TEST(TEST_CLASS, ReadFailsOnWriteError) {
		// Arrange: send one buffer containing one packet
		auto sendBuffer = test::GenerateRandomPacketBuffer(100, { 75, 50 });
		std::vector<ByteBuffer> sendBuffers{ sendBuffer };

		// Act: "server" - reads packets from the socket using the reader
		//               - closes the socket in the packet handler (before write) and then attempts to write
		//      "client" - writes sendBuffers to the socket
		ReaderFactory factory;
		ServerPacketHandlers handlers;
		SocketReadResult readResult;
		std::unique_ptr<SocketReader> pReader;
		test::SpawnPacketServerWork(factory.ioContext(), [&](const auto& pServerSocket) {
			test::RegisterDefaultHandler(handlers, [pServerSocket](const auto&, auto& context) {
				// - shut down the server socket
				pServerSocket->close();

				// - send a payload to the client to trigger a write
				CATAPULT_LOG(debug) << "sending byte to client";
				RespondWithBytes(context, {});
			});

			factory.startReader(pServerSocket, handlers, readResult, [&pReader](auto&& pStartedReader) {
				pReader = std::move(pStartedReader);
			});
		});
		auto pClientSocket = test::AddClientWriteBuffersTask(factory.ioContext(), sendBuffers);
		factory.join();

		// Assert: the server should get a write error when attempting to write to the client
		//         (the 25 unconsumed bytes are the remainder in the buffer after consuming the first packet)
		AssertSocketReadFailure(readResult, 1, SocketOperationCode::Write_Error, 25);
	}

	TEST(TEST_CLASS, ReadFailsOnMalformedPacket) {
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

	TEST(TEST_CLASS, CanChainReadOperations) {
		// Arrange: create two buffers
		auto sendBuffers = test::GenerateRandomPacketBuffers({ 20, 30 });

		// - set up a packet handler that copies the received packet bytes into receivedBuffers
		ReaderFactory factory;
		ServerPacketHandlers handlers;
		std::vector<ByteBuffer> receivedBuffers;
		test::AddCopyBuffersHandler(handlers, receivedBuffers);

		// Act: "server" - reads the first packet from the socket; signals; reads the second packet
		//      "client" - writes the first packet to the socket; waits; writes the second packet
		std::atomic<size_t> step(0);
		std::vector<std::vector<SocketOperationCode>> readCodes(2);
		std::unique_ptr<SocketReader> pReader;
		test::SpawnPacketServerWork(factory.ioContext(), [&](const auto& pServerSocket) {
			pReader = factory.createReader(pServerSocket, handlers);

			// - first read
			pReader->read([&readCodes, &step, &pReader](auto readCode1) {
				if (SocketOperationCode::Success == readCode1)
					++step;

				readCodes[0].push_back(readCode1);

				// - only chain the second read on a termination condition
				if (SocketOperationCode::Success == readCode1)
					return;

				// - second read
				pReader->read([&readCodes, &step](auto readCode2) {
					if (SocketOperationCode::Success == readCode2)
						++step;

					readCodes[1].push_back(readCode2);
				});
			});
		});
		test::CreateClientSocket(factory.ioContext())->connect().then([&](auto&& socketFuture) {
			auto* pClientSocket = socketFuture.get();
			pClientSocket->write(sendBuffers[0]).then([&, pClientSocket](auto) {
				WAIT_FOR_ONE(step); // wait for the first reader callback to be entered
				pClientSocket->write(sendBuffers[1]);

				WAIT_FOR_VALUE(2u, step); // wait for the second reader callback to be entered
				pClientSocket->shutdown();
			});
		});

		// - wait for the test to complete
		factory.join();

		// Assert: both buffers were read
		for (const auto& codes : readCodes) {
			ASSERT_EQ(2u, codes.size());
			EXPECT_EQ(SocketOperationCode::Success, codes[0]);
			EXPECT_EQ(SocketOperationCode::Insufficient_Data, codes[1]);
		}

		ASSERT_EQ(2u, receivedBuffers.size());
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 0, 20u, receivedBuffers[0]);
		EXPECT_EQUAL_BUFFERS(sendBuffers[1], 0, 30u, receivedBuffers[1]);
	}

	TEST(TEST_CLASS, CanCloseDuringRead) {
		// Act: "server" - starts a read and closes the socket
		//      "client" - connects to the server
		ReaderFactory factory;
		auto handlers = CreateNoOpHandlers();
		SocketOperationCode readCode;
		std::unique_ptr<SocketReader> pReader;
		test::SpawnPacketServerWork(factory.ioContext(), [&](const auto& pServerSocket) {
			pReader = factory.createReader(pServerSocket, handlers);
			pReader->read([&readCode](auto code) {
				readCode = code;
			});
			pServerSocket->close();
		});
		auto pClientSocket = test::AddClientConnectionTask(factory.ioContext());

		// - wait for the test to complete
		factory.join();

		// Assert: the socket was closed
		test::AssertSocketClosedDuringRead(readCode);
	}

	TEST(TEST_CLASS, CannotStartSimultaneousReads) {
		// Arrange: block the server handler until all reads have been attempted
		test::AutoSetFlag allReadsAttempted;
		auto pAllReadsAttempted = allReadsAttempted.state();
		ReaderFactory factory;
		ServerPacketHandlers handlers;
		handlers.registerHandler(test::Default_Packet_Type, [pAllReadsAttempted](const auto&, const auto&) {
			pAllReadsAttempted->wait();
		});

		auto sendBuffers = test::GenerateRandomPacketBuffers({ 100 });

		// Act: "server" - starts multiple reads
		//      "client" - writes a buffer to the socket
		std::unique_ptr<SocketReader> pReader;
		test::SpawnPacketServerWork(factory.ioContext(), [&, pAllReadsAttempted](const auto& pServerSocket) {
			pReader = factory.createReader(pServerSocket, handlers);
			pReader->read([](auto) {});

			// Act + Assert: attempting additional reads will throw
			EXPECT_THROW(pReader->read([](auto) {}), catapult_runtime_error);
			EXPECT_THROW(pReader->read([](auto) {}), catapult_runtime_error);
			pAllReadsAttempted->set();
		});
		auto pClientSocket = test::AddClientWriteBuffersTask(factory.ioContext(), sendBuffers);

		// - wait for the test to complete
		factory.join();
	}

	TEST(TEST_CLASS, AppropriateHandlerContextIsForwardedToHandlers) {
		// Arrange:
		auto clientPublicKey = test::GenerateRandomByteArray<Key>();
		auto clientHost = std::string("alice.com");
		model::NodeIdentity contextClientIdentity;

		ReaderFactory factory(clientPublicKey, clientHost);
		ServerPacketHandlers handlers;
		handlers.registerHandler(test::Default_Packet_Type, [&](const auto&, const auto& context) {
			// Act: save the context values
			contextClientIdentity = { context.key(), context.host() };
		});

		auto sendBuffers = test::GenerateRandomPacketBuffers({ 10 });

		// Act: "server" - reads packets from the socket using the reader
		//      "client" - writes sendBuffers to the socket
		SocketReadResult readResult;
		std::unique_ptr<SocketReader> pReader;
		factory.startReader(handlers, readResult, [&pReader](auto&& pStartedReader) {
			pReader = std::move(pStartedReader);
		});
		auto pClientSocket = test::AddClientWriteBuffersTask(factory.ioContext(), sendBuffers);

		// - wait for the test to complete
		factory.join();

		// Assert: context has expected values
		EXPECT_EQ(clientPublicKey, contextClientIdentity.PublicKey);
		EXPECT_EQ(clientHost, contextClientIdentity.Host);
	}
}}
