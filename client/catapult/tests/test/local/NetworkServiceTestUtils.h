#pragma once
#include "NetworkTestUtils.h"
#include "catapult/ionet/PacketSocket.h"
#include "catapult/local/LocalNodeStats.h"
#include "catapult/model/EntityInfo.h"
#include "catapult/net/VerifyPeer.h"
#include "catapult/thread/MultiServicePool.h"
#include "tests/test/core/AddressTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/core/TransactionTestUtils.h"
#include "tests/test/net/SocketTestUtils.h"
#include "tests/TestHarness.h"

namespace boost { namespace asio { class io_service; } }
namespace catapult {
	namespace ionet {
		struct Packet;
		class PacketIo;
	}
}

namespace catapult { namespace test {

	/// Generates a random push block packet.
	std::shared_ptr<ionet::Packet> GenerateRandomBlockPacket();

	/// Generates a random push transaction packet.
	std::shared_ptr<ionet::Packet> GenerateRandomTransactionPacket();

	// region reader service

	/// Asserts that the reader service can be booted and then calls \a handler.
	template<typename TTestContext, typename THandlerFunc>
	void AssertCanBootReaders(THandlerFunc handler) {
		// Arrange:
		TTestContext context;
		auto& service = context.service();

		// Act:
		context.boot();

		// Assert:
		EXPECT_EQ(0u, service.numActiveReaders());
		handler(context);
	}

	/// Asserts that the reader service can be shutdown and then calls \a handler.
	template<typename TTestContext, typename THandlerFunc>
	void AssertCanShutdownReaders(THandlerFunc handler) {
		// Arrange:
		TTestContext context;
		auto& service = context.service();
		context.boot();

		// Act:
		context.pool().shutdown();

		// Assert:
		EXPECT_EQ(local::Sentinel_Stats_Value, service.numActiveReaders());
		handler(context);
	}

	/// Creates a connection to localhost on the default port configured with server public key \a serverPublicKey
	/// using \a ioService and asserts that \a service has a single connected reader.
	template<typename TService>
	std::shared_ptr<ionet::PacketIo> AcceptReader(
			boost::asio::io_service& ioService,
			const TService& service,
			const Key& serverPublicKey) {
		// Act: connect to the server as a reader
		auto pIo = ConnectToLocalHost(ioService, Local_Host_Port, serverPublicKey);

		// Assert: a single connection was accepted
		EXPECT_EQ(1u, service.numActiveReaders());
		return pIo;
	}

	/// Accepts a single reader connection using \a context and then calls \a handler.
	template<typename TTestContext, typename THandlerFunc>
	void RunSingleReaderTest(TTestContext&& context, THandlerFunc handler) {
		// Arrange:
		auto& service = context.service();
		context.boot();

		// - connect to the server as a reader
		auto pPool = CreateStartedIoServiceThreadPool();
		auto pIo = AcceptReader(pPool->service(), service, context.publicKey());

		// Act + Assert: call the handler
		handler(context, service, *pIo);
	}

	// endregion

	// region writer service

	/// Asserts that the writer service can be booted and then calls \a handler.
	template<typename TTestContext, typename THandlerFunc>
	void AssertCanBootWriters(THandlerFunc handler) {
		// Arrange:
		TTestContext context;
		auto& service = context.service();

		// Act:
		context.boot();

		// Assert:
		EXPECT_EQ(0u, service.numActiveWriters());
		handler(service);
	}

	/// Asserts that the writer service can be shutdown and then calls \a handler.
	template<typename TTestContext, typename THandlerFunc>
	void AssertCanShutdownWriters(THandlerFunc handler) {
		// Arrange:
		TTestContext context;
		auto& service = context.service();
		context.boot();

		// Act:
		context.pool().shutdown();

		// Assert:
		EXPECT_EQ(local::Sentinel_Stats_Value, service.numActiveWriters());
		handler(service);
	}

	/// Creates a connection to localhost configured with server public key \a serverPublicKey,
	/// asserts that \a service has a single connected writer and then calls \a handler.
	template<typename TService, typename THandlerFunc>
	void ConnectToExternalWriter(TService& service, const Key& serverPublicKey, THandlerFunc handler) {
		// Act: get the packet writers and attempt to connect to the server
		ConnectToLocalHost(service.packetWriters(), serverPublicKey);

		// Assert: a single connection was accepted
		EXPECT_EQ(1u, service.numActiveWriters());
		handler(service);
	}

	/// Asserts that the writer service can accept a connection and then calls \a handler.
	template<typename TTestContext, typename THandlerFunc>
	void AssertWritersCanAcceptConnection(THandlerFunc handler) {
		// Arrange: create a (tcp) server
		auto pPool = CreateStartedIoServiceThreadPool();
		auto serverKeyPair = GenerateKeyPair();
		SpawnPacketServerWork(pPool->service(), [&serverKeyPair](const auto& pServer) {
			net::VerifyClient(pServer, serverKeyPair, [](auto, const auto&) {});
		});

		// - create the service
		TTestContext context;
		auto& service = context.service();
		context.boot();

		// Act + Assert: get the packet writers and attempt to connect to the server
		ConnectToExternalWriter(service, serverKeyPair.publicKey(), handler);
	}

	/// Coerces a packet \a buffer to an entity of the specified type.
	template<typename TEntity>
	const TEntity& CoercePacketToEntity(const ionet::ByteBuffer& buffer) {
		return *reinterpret_cast<const TEntity*>(buffer.data() + sizeof(ionet::PacketHeader));
	}

	/// Asserts that a transaction can be broadcast using the specified traits.
	template<typename TTraits>
	void AssertCanBroadcastTransaction() {
		// Arrange:
		auto pTransaction = std::shared_ptr<model::Transaction>(GenerateRandomTransaction());
		std::vector<model::TransactionInfo> infos;
		infos.push_back(model::TransactionInfo(pTransaction));

		// Assert:
		TTraits::AssertCanBroadcastEntity(
				[&infos](auto& service) { service.createNewTransactionsSink()(infos); },
				[&pTransaction](const auto& buffer) {
					// Assert: the server received the broadcasted entity
					EXPECT_EQ(*pTransaction, CoercePacketToEntity<model::Transaction>(buffer));
				});
	}

	/// Basic writers traits for broadcast tests.
	template<typename TTestContext, typename TConnector>
	struct BasicWritersTraits {
		template<typename TBroadcastEntity, typename TVerifyReadBuffer>
		static void AssertCanBroadcastEntity(TBroadcastEntity broadcastEntity, TVerifyReadBuffer verifyReadBuffer) {
			// Arrange: create a (tcp) server
			ionet::ByteBuffer packetBuffer;
			auto pPool = CreateStartedIoServiceThreadPool();
			auto serverKeyPair = GenerateKeyPair();
			SpawnPacketServerWork(pPool->service(), [&packetBuffer, &serverKeyPair](const auto& pServer) {
				// - verify the client
				net::VerifyClient(pServer, serverKeyPair, [&packetBuffer, pServer](auto, const auto&) {
					// - read the packet and copy it into packetBuffer
					AsyncReadIntoBuffer(*pServer, packetBuffer);
				});
			});

			// - create and boot the service
			TTestContext context;
			auto& service = context.service();
			context.boot();

			// - get the packet writers and attempt to connect to the server
			TConnector::ConnectToExternalWriter(service, serverKeyPair.publicKey());

			// Act: broadcast an entity to the server
			broadcastEntity(service);

			// - wait for the test to complete
			pPool->join();

			// Assert: the server received the broadcasted entity
			verifyReadBuffer(packetBuffer);
		}
	};

	// endregion
}}
