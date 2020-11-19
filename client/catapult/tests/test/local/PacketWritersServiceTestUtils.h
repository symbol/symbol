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

#pragma once
#include "NetworkTestUtils.h"
#include "ServiceLocatorTestContext.h"
#include "tests/test/core/BlockTestUtils.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/net/RemoteAcceptServer.h"

namespace catapult { namespace test {

	/// Traits for broadcasting a block.
	struct BlockBroadcastTraits {
		using EntityType = std::shared_ptr<model::Block>;

		static EntityType CreateEntity() {
			return EntityType(GenerateDeterministicBlock());
		}

		static void Broadcast(const EntityType& pBlock, const extensions::ServerHooks& hooks) {
			hooks.newBlockSink()(pBlock);
		}

		static void VerifyReadBuffer(const EntityType& pBlock, const ionet::ByteBuffer& packetBuffer) {
			EXPECT_EQ(*pBlock, CoercePacketToEntity<model::Block>(packetBuffer));
		}
	};

	/// Traits for broadcasting a transaction.
	struct TransactionBroadcastTraits {
		using EntityType = std::vector<model::TransactionInfo>;

		static EntityType CreateEntity() {
			EntityType transactionInfos;
			transactionInfos.push_back(model::TransactionInfo(GenerateRandomTransaction()));
			return transactionInfos;
		}

		static void Broadcast(const EntityType& transactionInfos, const extensions::ServerHooks& hooks) {
			hooks.newTransactionsSink()(transactionInfos);
		}

		static void VerifyReadBuffer(const EntityType& transactionInfos, const ionet::ByteBuffer& packetBuffer) {
			EXPECT_EQ(*transactionInfos[0].pEntity, CoercePacketToEntity<model::Transaction>(packetBuffer));
		}
	};

	/// Grouping of basic packet writers service tests.
	template<typename TMixin>
	struct PacketWritersServiceTests {
	private:
		using Traits = typename TMixin::TraitsType;
		using TestContext = typename TMixin::TestContextType;

	public:
		// region boot / shutdown

		static void AssertCanBootService() {
			// Arrange:
			TestContext context;

			// Act:
			context.boot();

			// Assert:
			EXPECT_EQ(Traits::Num_Expected_Services, context.locator().numServices());
			EXPECT_EQ(1u, context.locator().counters().size());

			EXPECT_TRUE(!!Traits::GetWriters(context.locator()));
			EXPECT_EQ(0u, context.counter(Traits::Counter_Name));
		}

		static void AssertCanShutdownService() {
			// Arrange:
			TestContext context;

			// Act:
			context.boot();
			context.shutdown();

			// Assert:
			EXPECT_EQ(Traits::Num_Expected_Services, context.locator().numServices());
			EXPECT_EQ(1u, context.locator().counters().size());

			EXPECT_FALSE(!!Traits::GetWriters(context.locator()));
			EXPECT_EQ(extensions::ServiceLocator::Sentinel_Counter_Value, context.counter(Traits::Counter_Name));
		}

		// endregion

	private:
		// region ServerBootstrapper

		class ServerBootstrapper : public RemoteAcceptServer {
		public:
			void connectToExternalWriter(const TestContext& context) {
				// Act: get the packet writers and attempt to connect to the server
				ConnectToLocalHost(*Traits::GetWriters(context.locator()), caPublicKey());

				// Assert: a single connection was accepted
				EXPECT_EQ(1u, context.counter(Traits::Counter_Name));
			}
		};

		// endregion

	public:
		// region connection + writing

		static void AssertCanConnectToExternalServer() {
			// Arrange: create a (tcp) server
			ServerBootstrapper server;
			server.start();

			// - create the service
			TestContext context;
			context.boot();

			// Act + Assert: attempt to connect to the server
			server.connectToExternalWriter(context);
		}

		static void AssertCanBroadcastBlockToWriters() {
			// Assert:
			AssertCanBroadcastEntityToWriters<BlockBroadcastTraits>();
		}

		static void AssertCanBroadcastTransactionToWriters() {
			// Assert:
			AssertCanBroadcastEntityToWriters<TransactionBroadcastTraits>();
		}

	private:
		template<typename TBroadcastTraits>
		static void AssertCanBroadcastEntityToWriters() {
			// Arrange: create a (tcp) server
			ServerBootstrapper server;
			ionet::ByteBuffer packetBuffer;
			server.start([&ioContext = server.ioContext(), &packetBuffer](const auto& pServer) {
				// - read the packet and copy it into packetBuffer
				AsyncReadIntoBuffer(ioContext, *pServer, packetBuffer);
			});

			// - create and boot the service
			TestContext context;
			context.boot();

			// - get the packet writers and attempt to connect to the server
			server.connectToExternalWriter(context);

			// Act: broadcast an entity to the server
			auto entity = TBroadcastTraits::CreateEntity();
			TBroadcastTraits::Broadcast(entity, context.testState().state().hooks());

			// - wait for the test to complete
			server.join();

			// Assert: the server received the broadcasted entity
			ASSERT_FALSE(packetBuffer.empty());
			TBroadcastTraits::VerifyReadBuffer(entity, packetBuffer);
		}

		// endregion

	public:
		// region bannedNodeIdentitySink

		static void AssertWritersAreRegisteredInBannedNodeIdentitySink() {
			// Arrange: create a (tcp) server
			ServerBootstrapper server;
			server.start();

			// Act: create and boot the service
			TestContext context;
			context.boot();
			auto sink = context.testState().state().hooks().bannedNodeIdentitySink();

			// - get the packet writers and attempt to connect to the server
			server.connectToExternalWriter(context);

			// Act: trigger the sink, which should close the connection
			sink(model::NodeIdentity{ server.caPublicKey(), "" });

			// - wait for the test to complete
			server.join();

			// Assert: the connection was closed
			EXPECT_EQ(0u, context.counter(Traits::Counter_Name));
		}

		// endregion
	};

#define ADD_PACKET_WRITERS_SERVICE_TEST(TEST_CLASS, MIXIN, TEST_NAME) \
	TEST(TEST_CLASS, TEST_NAME) { test::PacketWritersServiceTests<MIXIN>::Assert##TEST_NAME(); }

#define DEFINE_PACKET_WRITERS_SERVICE_TESTS(TEST_CLASS, MIXIN) \
	ADD_PACKET_WRITERS_SERVICE_TEST(TEST_CLASS, MIXIN, CanBootService) \
	ADD_PACKET_WRITERS_SERVICE_TEST(TEST_CLASS, MIXIN, CanShutdownService) \
	ADD_PACKET_WRITERS_SERVICE_TEST(TEST_CLASS, MIXIN, CanConnectToExternalServer) \
	ADD_PACKET_WRITERS_SERVICE_TEST(TEST_CLASS, MIXIN, CanBroadcastBlockToWriters) \
	ADD_PACKET_WRITERS_SERVICE_TEST(TEST_CLASS, MIXIN, CanBroadcastTransactionToWriters) \
	ADD_PACKET_WRITERS_SERVICE_TEST(TEST_CLASS, MIXIN, WritersAreRegisteredInBannedNodeIdentitySink)
}}
