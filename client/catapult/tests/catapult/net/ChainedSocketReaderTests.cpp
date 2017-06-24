#include "catapult/net/ChainedSocketReader.h"
#include "catapult/ionet/BufferedPacketIo.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/ionet/SocketReader.h"
#include "catapult/net/AsyncTcpServer.h"
#include "catapult/thread/IoServiceThreadPool.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/net/ClientSocket.h"
#include "tests/test/net/SocketTestUtils.h"
#include "tests/test/net/mocks/MockAsyncTcpServerAcceptContext.h"

using catapult::mocks::MockAsyncTcpServerAcceptContext;

namespace catapult { namespace net {

	namespace {
		std::shared_ptr<ChainedSocketReader> CreateChainedReader(
				boost::asio::io_service& service,
				const std::shared_ptr<ionet::PacketSocket>& pSocket,
				const ionet::ServerPacketHandlers& handlers,
				ionet::SocketOperationCode& completionCode) {
			auto pAcceptContext = std::make_shared<MockAsyncTcpServerAcceptContext>(service, pSocket);
			return CreateChainedSocketReader(pAcceptContext, handlers, [&completionCode](auto code) {
				completionCode = code;
			});
		}

		/// Options for configuring SendBuffers.
		struct SendBuffersOptions {
			SendBuffersOptions()
					: NumReadsToConfirm(0)
					, HookPacketReceived([](const auto&) {})
			{}

			/// The number of reads to confirm.
			size_t NumReadsToConfirm;

			/// Hook that is passed every packet as it is read.
			std::function<void (ChainedSocketReader&)> HookPacketReceived;
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
					return;
				}

				CATAPULT_LOG(debug) << "writing buffer " << id << " / " << sendBuffers.size();
				socket.write(sendBuffers[id - 1]).then([&, id](auto&& writeFuture) {
					writeFuture.get(); // fail if the write failed

					if (id <= m_numReadsToConfirm) {
						CATAPULT_LOG(debug) << "confirming receipt of buffer " << id;
						WAIT_FOR_VALUE(m_numReceivedBuffers, id);
					}

					this->write(socket, sendBuffers, id);
				});
			}

		private:
			std::atomic<size_t>& m_numReceivedBuffers;
			size_t m_numReadsToConfirm;
		};

		/// Writes all \a sendBuffers to a socket and reads them with a reader.
		std::pair<std::vector<ionet::ByteBuffer>, ionet::SocketOperationCode> SendBuffers(
				const std::vector<ionet::ByteBuffer>& sendBuffers,
				SendBuffersOptions options = SendBuffersOptions()) {
			// Arrange:
			ionet::ServerPacketHandlers handlers;
			std::atomic<size_t> numReceivedBuffers(0);
			std::vector<ionet::ByteBuffer> receivedBuffers;

			// Act: "server" - reads packets from the socket using the Reader
			//      "client" - writes sendBuffers to the socket waiting until the previous buffer has been read
			auto pPool = test::CreateStartedIoServiceThreadPool();
			std::weak_ptr<ChainedSocketReader> pReader;
			ionet::SocketOperationCode completionCode;
			test::SpawnPacketServerWork(pPool->service(), [&](const auto& pServerSocket) -> void {
				auto pReaderShared = CreateChainedReader(pPool->service(), pServerSocket, handlers, completionCode);
				pReader = pReaderShared;

				// Arrange: set up a packet handler that copies the received packet bytes into receivedBuffers
				test::RegisterDefaultHandler(handlers, [&](const auto& packet, const auto&) {
					CATAPULT_LOG(debug) << "server received data " << packet.Size;
					receivedBuffers.push_back(test::CopyPacketToBuffer(packet));

					// if this handler is called, the reader must still be valid, so locking the weak_ptr is safe
					options.HookPacketReceived(*pReader.lock());

					++numReceivedBuffers;
				});

				CATAPULT_LOG(debug) << "started reader ";
				pReaderShared->start();
			});

			auto pWriteContext = std::make_shared<WriteHandshakeContext>(numReceivedBuffers, options.NumReadsToConfirm);
			test::CreateClientSocket(pPool->service())->connect().then([&](auto&& socketFuture) {
				pWriteContext->start(*socketFuture.get(), sendBuffers);
			});

			// - wait for the test to complete (indicated by the destruction of the reader)
			WAIT_FOR_EXPR(nullptr == pReader.lock());
			pPool->join();

			return std::make_pair(receivedBuffers, completionCode);
		}
	}

	TEST(ChainedSocketReaderTests, ReaderIsNotAutoStarted) {
		// Arrange:
		std::atomic<int> numReads(0);
		ionet::ServerPacketHandlers handlers;
		test::RegisterDefaultHandler(handlers, [&numReads](const auto&, const auto&) {
			++numReads;
		});

		// Act: "server" - creates a chained reader but does not start it
		//      "client" - sends a packet to the server
		auto pPool = test::CreateStartedIoServiceThreadPool();
		auto completionCode = static_cast<ionet::SocketOperationCode>(123);
		std::shared_ptr<ChainedSocketReader> pReader;
		test::SpawnPacketServerWork(pPool->service(), [&](const auto& pServerSocket) -> void {
			pReader = CreateChainedReader(pPool->service(), pServerSocket, handlers, completionCode);
		});
		test::AddClientWriteBuffersTask(pPool->service(), { test::GenerateRandomPacketBuffer(50) });

		// - wait for the test to complete
		pPool->join();

		// Assert: no reads took place and the chain was never completed (because it was never started)
		EXPECT_EQ(0u, numReads);
		EXPECT_EQ(static_cast<ionet::SocketOperationCode>(123), completionCode);
	}

	TEST(ChainedSocketReaderTests, ReaderCanReadSinglePacket) {
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

	TEST(ChainedSocketReaderTests, ReaderCanReadMultiplePackets) {
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

	TEST(ChainedSocketReaderTests, ReaderStopsAfterError) {
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

	TEST(ChainedSocketReaderTests, ReaderCanBeStopped) {
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

		// Assert: only the first packet (before the reader was stopped) was read
		EXPECT_EQ(ionet::SocketOperationCode::Read_Error, completionCode);
		ASSERT_EQ(1u, receivedBuffers.size());
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 0, 20u, receivedBuffers[0]);
	}

	TEST(ChainedSocketReaderTests, ReadsAreChained) {
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
		EXPECT_EQ(ionet::SocketOperationCode::Closed, completionCode);
		ASSERT_EQ(5u, receivedBuffers.size());
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 0, 20u, receivedBuffers[0]);
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 20, 17u, receivedBuffers[1]);
		EXPECT_EQUAL_BUFFERS(sendBuffers[0], 37, 50u, receivedBuffers[2]);
		EXPECT_EQUAL_BUFFERS(sendBuffers[1], 0, 1024u, receivedBuffers[3]);
		EXPECT_EQUAL_BUFFERS(sendBuffers[1], 1024, 2048u, receivedBuffers[4]);
	}
}}
