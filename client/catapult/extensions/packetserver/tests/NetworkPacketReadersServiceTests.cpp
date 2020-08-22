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

#include "packetserver/src/NetworkPacketReadersService.h"
#include "catapult/handlers/BasicProducer.h"
#include "catapult/handlers/HandlerFactory.h"
#include "tests/test/core/ThreadPoolTestUtils.h"
#include "tests/test/local/NetworkTestUtils.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace packetserver {

#define TEST_CLASS NetworkPacketReadersServiceTests

	namespace {
		constexpr auto Counter_Name = "READERS";
		constexpr auto Service_Name = "readers";

		struct NetworkPacketReadersServiceTraits {
			static constexpr auto CreateRegistrar = CreateNetworkPacketReadersServiceRegistrar;
		};

		using TestContext = test::ServiceLocatorTestContext<NetworkPacketReadersServiceTraits>;
	}

	ADD_SERVICE_REGISTRAR_INFO_TEST(NetworkPacketReaders, Post_Packet_Handlers)

	// region boot + shutdown

	TEST(TEST_CLASS, CanBootService) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot();

		// Assert:
		EXPECT_EQ(1u, context.locator().numServices());
		EXPECT_EQ(1u, context.locator().counters().size());

		EXPECT_TRUE(!!context.locator().service<net::PacketReaders>(Service_Name));
		EXPECT_EQ(0u, context.counter(Counter_Name));
	}

	TEST(TEST_CLASS, CanShutdownService) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot();
		context.shutdown();

		// Assert:
		EXPECT_EQ(1u, context.locator().numServices());
		EXPECT_EQ(1u, context.locator().counters().size());

		EXPECT_FALSE(!!context.locator().service<net::PacketReaders>(Service_Name));
		EXPECT_EQ(static_cast<uint64_t>(extensions::ServiceLocator::Sentinel_Counter_Value), context.counter(Counter_Name));
	}

	// endregion

	// region connections

	TEST(TEST_CLASS, CanAcceptExternalConnection) {
		// Arrange:
		TestContext context;
		context.boot();

		// - connect to the server as a reader
		auto pPool = test::CreateStartedIoThreadPool();
		auto pIo = test::ConnectToLocalHost(pPool->ioContext(), test::GetLocalHostPort());

		// - wait for a single connection
		WAIT_FOR_ONE_EXPR(context.counter(Counter_Name));

		// Assert: a single connection was accepted
		EXPECT_EQ(1u, context.counter(Counter_Name));
	}

	// endregion

	// region handlers

	namespace {
		struct SquaresTraits {
			using RequestStructureType = uint64_t;

			static constexpr auto Packet_Type = static_cast<ionet::PacketType>(25);
			static constexpr auto Should_Append_As_Values = true;

			class Producer : public handlers::BasicProducer<model::EntityRange<uint64_t>> {
			public:
				using BasicProducer<model::EntityRange<uint64_t>>::BasicProducer;

			public:
				auto operator()() {
					return next([](auto value) {
						// 1. square the value
						// 2. return a shared_ptr because a pointer is expected to be returned
						return std::make_shared<uint64_t>(value * value);
					});
				}
			};
		};

		std::shared_ptr<ionet::Packet> GenerateSquaresRequestPacket() {
			auto pPacket = ionet::CreateSharedPacket<ionet::Packet>(3 * sizeof(uint64_t));
			pPacket->Type = SquaresTraits::Packet_Type;

			auto* pData = reinterpret_cast<uint64_t*>(pPacket->Data());
			pData[0] = 3;
			pData[1] = 8;
			pData[2] = 5;
			return pPacket;
		}
	}

	TEST(TEST_CLASS, CanBootServiceWithArbitraryHandlers) {
		// Arrange:
		TestContext context;
		auto& packetHandlers = context.testState().state().packetHandlers();

		// - register a single handler
		handlers::BatchHandlerFactory<SquaresTraits>::RegisterOne(packetHandlers, [](const auto& values) {
			return SquaresTraits::Producer(values);
		});

		context.boot();

		// - connect to the server as a reader
		auto pPool = test::CreateStartedIoThreadPool();
		auto pIo = test::ConnectToLocalHost(pPool->ioContext(), test::GetLocalHostPort());

		// - wait for a single connection
		WAIT_FOR_ONE_EXPR(context.counter(Counter_Name));

		// Act: send a simple squares request
		ionet::ByteBuffer packetBuffer;
		auto pRequestPacket = GenerateSquaresRequestPacket();
		pIo->write(ionet::PacketPayload(pRequestPacket), [&ioContext = pPool->ioContext(), &io = *pIo, &packetBuffer](auto) {
			test::AsyncReadIntoBuffer(ioContext, io, packetBuffer);
		});

		pPool->join();

		// Assert: the requested squared values should have been returned
		auto pResponsePacket = reinterpret_cast<const ionet::PacketHeader*>(packetBuffer.data());
		ASSERT_TRUE(!!pResponsePacket);
		ASSERT_EQ(sizeof(ionet::PacketHeader) + 3 * sizeof(uint64_t), pResponsePacket->Size);
		EXPECT_EQ(SquaresTraits::Packet_Type, pResponsePacket->Type);

		const auto* pData = reinterpret_cast<const uint64_t*>(packetBuffer.data() + sizeof(ionet::PacketHeader));
		EXPECT_EQ(9u, pData[0]);
		EXPECT_EQ(64u, pData[1]);
		EXPECT_EQ(25u, pData[2]);
	}

	// endregion

	// region bannedNodeIdentitySink

	TEST(TEST_CLASS, ReadersAreRegisteredInBannedNodeIdentitySink) {
		// Arrange:
		TestContext context;
		context.boot();
		auto sink = context.testState().state().hooks().bannedNodeIdentitySink();

		// - connect to the server as a reader
		auto pPool = test::CreateStartedIoThreadPool();
		auto pIo = test::ConnectToLocalHost(pPool->ioContext(), test::GetLocalHostPort());

		// Sanity: a single connection was accepted
		WAIT_FOR_ONE_EXPR(context.counter(Counter_Name));

		// - figure out the identity of the connected reader
		auto pReaders = context.locator().service<net::PacketReaders>(Service_Name);
		auto clientPublicKey = pReaders->identities().cbegin()->PublicKey;

		// Act: trigger the sink, which should close the connection
		sink(model::NodeIdentity{ clientPublicKey, "" });

		// - wait for the test to complete
		pPool->join();

		// Assert: the connection was closed
		EXPECT_EQ(0u, context.counter(Counter_Name));
	}

	// endregion

	// region tasks

	TEST(TEST_CLASS, TasksAreRegistered) {
		test::AssertRegisteredTasks(TestContext(), { "age peers task for service Readers" });
	}

	// endregion
}}
