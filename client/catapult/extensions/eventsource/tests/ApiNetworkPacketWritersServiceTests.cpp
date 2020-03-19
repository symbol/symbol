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

#include "eventsource/src/ApiNetworkPacketWritersService.h"
#include "tests/test/local/PacketWritersServiceTestUtils.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace eventsource {

#define TEST_CLASS ApiNetworkPacketWritersServiceTests

	namespace {
		struct ApiNetworkPacketWritersServiceTraits {
			static constexpr auto Counter_Name = "B WRITERS";
			static constexpr auto Num_Expected_Services = 1u;

			static auto GetWriters(const extensions::ServiceLocator& locator) {
				return locator.service<net::PacketWriters>("api.writers");
			}

			static constexpr auto CreateRegistrar = CreateApiNetworkPacketWritersServiceRegistrar;
		};

		using TestContext = test::ServiceLocatorTestContext<ApiNetworkPacketWritersServiceTraits>;

		struct Mixin {
			using TraitsType = ApiNetworkPacketWritersServiceTraits;
			using TestContextType = TestContext;
		};
	}

	ADD_SERVICE_REGISTRAR_INFO_TEST(ApiNetworkPacketWriters, Initial)

	DEFINE_PACKET_WRITERS_SERVICE_TESTS(TEST_CLASS, Mixin)

	// region accepting external conncetions

	namespace {
		std::shared_ptr<ionet::PacketSocket> AcceptBroadcastWriter(boost::asio::io_context& ioContext, const TestContext& context) {
			// Act: connect to the server as a broadcast writer
			unsigned short localHostApiPort = test::GetLocalHostPort() + 1;
			auto pIo = test::ConnectToLocalHost(ioContext, localHostApiPort);

			// - wait for the writer to be available
			WAIT_FOR_ONE_EXPR(context.counter("B WRITERS"));

			// - wait a bit in case there are more connects to be served if there is a bug in the implementation
			test::Pause();

			// Assert: a single connection was accepted
			EXPECT_EQ(1u, context.counter("B WRITERS"));
			return pIo;
		}
	}

	TEST(TEST_CLASS, CanAcceptExternalConnection) {
		// Arrange: create and boot the service
		TestContext context;
		context.boot();

		// Act + Assert: connect to the server as a broadcast writer
		auto pPool = test::CreateStartedIoThreadPool();
		AcceptBroadcastWriter(pPool->ioContext(), context);
	}

	namespace {
		template<typename TBroadcastTraits>
		static void AssertCanBroadcastEntityToAcceptedConnections() {
			// Arrange: create and boot the service
			TestContext context;
			context.boot();

			// - connect to the server as a broadcast writer
			auto pPool = test::CreateStartedIoThreadPool();
			auto pIo = AcceptBroadcastWriter(pPool->ioContext(), context);

			// - set up a read
			ionet::ByteBuffer packetBuffer;
			test::AsyncReadIntoBuffer(pPool->ioContext(), *pIo, packetBuffer);

			// Act: broadcast an entity to the server
			auto entity = TBroadcastTraits::CreateEntity();
			TBroadcastTraits::Broadcast(entity, context.testState().state().hooks());

			// - wait for the test to complete
			pPool->join();

			// Assert: the external connection received the broadcasted entity
			ASSERT_FALSE(packetBuffer.empty());
			TBroadcastTraits::VerifyReadBuffer(entity, packetBuffer);
		}
	}

	TEST(TEST_CLASS, CanBroadcastBlockToAcceptedConnections) {
		AssertCanBroadcastEntityToAcceptedConnections<test::BlockBroadcastTraits>();
	}

	TEST(TEST_CLASS, CanBroadcastTransactionToAcceptedConnections) {
		AssertCanBroadcastEntityToAcceptedConnections<test::TransactionBroadcastTraits>();
	}

	// endregion

	// region tasks

	TEST(TEST_CLASS, AgePeersTaskIsScheduled) {
		test::AssertRegisteredTask(TestContext(), 1, "age peers task for service Api Writers");
	}

	// endregion
}}
