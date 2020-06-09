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

#include "catapult/net/ChainedSocketReader.h"
#include "catapult/ionet/BufferedPacketIo.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/ionet/SocketReader.h"
#include "catapult/thread/IoThreadPool.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/net/ClientSocket.h"
#include "tests/test/net/SocketTestUtils.h"

namespace catapult { namespace net {

#define TEST_CLASS ChainedSocketReaderTests

	namespace {
		// reference inside chained reader is actually wanted, so use alias not to trigger lint warning
		using ChainedReaderCompletionCode = ionet::SocketOperationCode;

		std::shared_ptr<ChainedSocketReader> CreateChainedReader(
				const std::shared_ptr<ionet::PacketSocket>& pSocket,
				const ionet::ServerPacketHandlers& handlers,
				const model::NodeIdentity& identity,
				ChainedReaderCompletionCode& completionCode) {
			return CreateChainedSocketReader(pSocket, handlers, identity, [&completionCode](auto code) {
				completionCode = code;
			});
		}

		// options for configuring SendBuffers
		struct SendBuffersOptions {
		public:
			SendBuffersOptions()
					: NumReadsToConfirm(0)
					, HookPacketReceived([](const auto&) {})
			{}

		public:
			// number of reads to confirm
			size_t NumReadsToConfirm;

			// hook that is passed every packet as it is read
			consumer<ChainedSocketReader&> HookPacketReceived;
		};

		class WriteHandshakeContext {
		public:
			WriteHandshakeContext(std::atomic<size_t>& numReceivedBuffers, size_t numReadsToConfirm)
					: m_numReceivedBuffers(numReceivedBuffers)
					, m_numReadsToConfirm(numReadsToConfirm)
			{}

		public:
			void start(test::ClientSocket& socket, const std::vector<ionet::ByteBuffer>& sendBuffers) {
				write(socket, sendBuffers, 0);
			}

		private:
			void write(test::ClientSocket& socket, const std::vector<ionet::ByteBuffer>& sendBuffers, size_t nextId) {
				auto id = ++nextId;
				if (id > sendBuffers.size()) {
					CATAPULT_LOG(debug) << "wrote " << sendBuffers.size() << " buffers";

					// don't shutdown unless at least one buffer was received in order to ensure all ssl handshakes are properly completed
					WAIT_FOR_EXPR(m_numReceivedBuffers >= 1);
					socket.shutdown();
					return;
				}

				CATAPULT_LOG(debug) << "writing buffer " << id << " / " << sendBuffers.size();
				socket.write(sendBuffers[id - 1]).then([&, id](auto&& writeFuture) {
					writeFuture.get(); // fail if the write failed

					if (id <= m_numReadsToConfirm) {
						CATAPULT_LOG(debug) << "confirming receipt of buffer " << id;
						WAIT_FOR_VALUE(id, m_numReceivedBuffers);
					}

					this->write(socket, sendBuffers, id);
				});
			}

		private:
			std::atomic<size_t>& m_numReceivedBuffers;
			size_t m_numReadsToConfirm;
		};

		model::NodeIdentity CreateDefaultClientIdentity() {
			return { test::GenerateRandomByteArray<Key>(), "alice.com" };
		}

		// writes all \a sendBuffers to a socket and reads them with a reader
		std::pair<std::vector<ionet::ByteBuffer>, ionet::SocketOperationCode> SendBuffers(
				const std::vector<ionet::ByteBuffer>& sendBuffers,
				SendBuffersOptions options = SendBuffersOptions()) {
			// Arrange:
			ionet::ServerPacketHandlers handlers;
			auto clientIdentity = CreateDefaultClientIdentity();
			std::atomic<size_t> numReceivedBuffers(0);
			std::vector<ionet::ByteBuffer> receivedBuffers;

			// Act: "server" - reads packets from the socket using the Reader
			//      "client" - writes sendBuffers to the socket waiting until the previous buffer has been read
			auto pPool = test::CreateStartedIoThreadPool();
			std::weak_ptr<ChainedSocketReader> pReader;
			ionet::SocketOperationCode completionCode;
			test::SpawnPacketServerWork(pPool->ioContext(), [&](const auto& pServerSocket) {
				auto pReaderShared = CreateChainedReader(pServerSocket, handlers, clientIdentity, completionCode);
				pReader = pReaderShared;

				// Arrange: set up a packet handler that copies the received packet bytes into receivedBuffers
				test::RegisterDefaultHandler(handlers, [&](const auto& packet, const auto&) {
					CATAPULT_LOG(debug) << "server received data: " << packet;
					receivedBuffers.push_back(test::CopyPacketToBuffer(packet));

					// if this handler is called, the reader must still be valid, so locking the weak_ptr is safe
					options.HookPacketReceived(*pReader.lock());

					++numReceivedBuffers;
				});

				CATAPULT_LOG(debug) << "started reader ";
				pReaderShared->start();
			});

			auto pWriteContext = std::make_shared<WriteHandshakeContext>(numReceivedBuffers, options.NumReadsToConfirm);
			auto pClientSocket = test::CreateClientSocket(pPool->ioContext());
			pClientSocket->connect().then([&](auto&& socketFuture) {
				pWriteContext->start(*socketFuture.get(), sendBuffers);
			});

			// - wait for the test to complete (indicated by the destruction of the reader)
			WAIT_FOR_EXPR(!pReader.lock());
			pPool->join();

			return std::make_pair(receivedBuffers, completionCode);
		}
	}

	TEST(TEST_CLASS, ReaderIsNotAutoStarted) {
		// Arrange:
		std::atomic<uint32_t> numReads(0);
		auto clientIdentity = CreateDefaultClientIdentity();
		ionet::ServerPacketHandlers handlers;
		test::RegisterDefaultHandler(handlers, [&numReads](const auto&, const auto&) {
			++numReads;
		});

		// Act: "server" - creates a chained reader but does not start it
		//      "client" - sends a packet to the server
		auto pPool = test::CreateStartedIoThreadPool();
		auto completionCode = static_cast<ionet::SocketOperationCode>(123);
		std::shared_ptr<ChainedSocketReader> pReader;
		test::SpawnPacketServerWork(pPool->ioContext(), [&](const auto& pServerSocket) {
			pReader = CreateChainedReader(pServerSocket, handlers, clientIdentity, completionCode);
		});
		auto pClientSocket = test::AddClientWriteBuffersTask(pPool->ioContext(), { test::GenerateRandomPacketBuffer(50) });

		// - wait for the test to complete
		pPool->join();

		// Assert: no reads took place and the chain was never completed (because it was never started)
		EXPECT_EQ(0u, numReads);
		EXPECT_EQ(static_cast<ionet::SocketOperationCode>(123), completionCode);
	}

	TEST(TEST_CLASS, AppropriateHandlerContextIsForwardedToHandlers) {
		// Arrange:
		auto clientIdentity = CreateDefaultClientIdentity();
		model::NodeIdentity contextClientIdentity;

		ionet::ServerPacketHandlers handlers;
		test::RegisterDefaultHandler(handlers, [&contextClientIdentity](const auto&, const auto& context) {
			// Act: save the context values
			contextClientIdentity = { context.key(), context.host() };
		});

		// Act: "server" - creates a chained reader and starts it
		//      "client" - sends a packet to the server
		auto pPool = test::CreateStartedIoThreadPool();
		auto completionCode = static_cast<ionet::SocketOperationCode>(123);
		std::shared_ptr<ChainedSocketReader> pReader;
		test::SpawnPacketServerWork(pPool->ioContext(), [&](const auto& pServerSocket) {
			pReader = CreateChainedReader(pServerSocket, handlers, clientIdentity, completionCode);
			pReader->start();
		});
		auto pClientSocket = test::AddClientWriteBuffersTask(pPool->ioContext(), { test::GenerateRandomPacketBuffer(50) });
		pClientSocket.reset(); // server socket does not read the data

		// - wait for the test to complete
		pPool->join();

		// Assert: context has expected values
		EXPECT_EQ(clientIdentity.PublicKey, contextClientIdentity.PublicKey);
		EXPECT_EQ(clientIdentity.Host, contextClientIdentity.Host);
	}

	TEST(TEST_CLASS, ReaderCanReadSinglePacket) {
		// Arrange: send a single packet
		auto sendBuffer = test::GenerateRandomPacketBuffer(100, { 82, 75 });
		std::vector<ionet::ByteBuffer> sendBuffers{ sendBuffer };

		// Act:
		auto resultPair = SendBuffers(sendBuffers);
		const auto& receivedBuffers = resultPair.first;
		auto completionCode = resultPair.second;

		// Assert: the single packet was successfuly read
		EXPECT_EQ(ionet::SocketOperationCode::Closed, completionCode);
		ASSERT_EQ(1u, receivedBuffers.size());
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 0, 82u, receivedBuffers[0]);
	}

	TEST(TEST_CLASS, ReaderCanReadMultiplePackets) {
		// Arrange: send three packets
		auto sendBuffers = test::GenerateRandomPacketBuffers({ 20, 17, 50 });

		// Act:
		auto resultPair = SendBuffers(sendBuffers);
		const auto& receivedBuffers = resultPair.first;
		auto completionCode = resultPair.second;

		// Assert: all packets were successfully read
		EXPECT_EQ(ionet::SocketOperationCode::Closed, completionCode);
		ASSERT_EQ(3u, receivedBuffers.size());
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 0, 20u, receivedBuffers[0]);
		EXPECT_EQUAL_BUFFERS(sendBuffers[1], 0, 17u, receivedBuffers[1]);
		EXPECT_EQUAL_BUFFERS(sendBuffers[2], 0, 50u, receivedBuffers[2]);
	}

	TEST(TEST_CLASS, ReaderStopsAfterError) {
		// Arrange: send a buffer containing three packets with the second one malformed
		auto sendBuffers = test::GenerateRandomPacketBuffers({ 20, 17, 50 });
		test::SetPacketAt(sendBuffers[1], 0, 0);

		// Act:
		auto resultPair = SendBuffers(sendBuffers);
		const auto& receivedBuffers = resultPair.first;
		auto completionCode = resultPair.second;

		// Assert: only the packet before the malformed one was consumed and consumption was stopped
		EXPECT_EQ(ionet::SocketOperationCode::Malformed_Data, completionCode);
		ASSERT_EQ(1u, receivedBuffers.size());
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 0, 20u, receivedBuffers[0]);
	}

	TEST(TEST_CLASS, ReaderCanBeStopped) {
		// Assert: non-deterministic because stop is async
		test::RunNonDeterministicTest("ReaderCanBeStopped", []() {
			// Arrange: send three packets
			auto sendBuffers = test::GenerateRandomPacketBuffers({ 20, 17, 50 });

			// Act: stop the reader after the first packet was read
			//      (this test needs to confirm the first read in order to delay the subsequent writes
			//       until AFTER the first read is handled and the socket is closed)
			SendBuffersOptions options;
			options.NumReadsToConfirm = 1;
			options.HookPacketReceived = [](auto& reader) {
				CATAPULT_LOG(debug) << "stopping reader ";
				reader.stop();
				CATAPULT_LOG(debug) << "stopped reader ";
			};
			auto resultPair = SendBuffers(sendBuffers, options);
			const auto& receivedBuffers = resultPair.first;
			auto completionCode = resultPair.second;

			if (1 < receivedBuffers.size())
				return false;

			// Assert: only the first packet (before the reader was stopped) was read
			EXPECT_EQ(ionet::SocketOperationCode::Read_Error, completionCode);
			EXPECT_EQ(1u, receivedBuffers.size());
			EXPECT_EQUAL_BUFFERS(sendBuffers[0], 0, 20u, receivedBuffers[0]);
			return true;
		});
	}

	TEST(TEST_CLASS, ReadsAreChained) {
		// Arrange: send two multi-packet buffers
		std::vector<ionet::ByteBuffer> sendBuffers{
			test::GenerateRandomPacketBuffer(87, { 20, 17, 50 }),
			test::GenerateRandomPacketBuffer(3072, { 1024, 2048 })
		};

		// Act:
		auto resultPair = SendBuffers(sendBuffers);
		const auto& receivedBuffers = resultPair.first;
		auto completionCode = resultPair.second;

		// Assert: all packets were successfully read
		test::AssertSocketClosedDuringRead(completionCode);
		ASSERT_EQ(5u, receivedBuffers.size());
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 0, 20u, receivedBuffers[0]);
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 20, 17u, receivedBuffers[1]);
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 37, 50u, receivedBuffers[2]);
		EXPECT_EQUAL_BUFFERS(sendBuffers[1], 0, 1024u, receivedBuffers[3]);
		EXPECT_EQUAL_BUFFERS(sendBuffers[1], 1024, 2048u, receivedBuffers[4]);
	}
}}
