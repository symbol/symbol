#include "packetserver/src/NetworkPacketReadersService.h"
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
		auto pPool = test::CreateStartedIoServiceThreadPool();
		auto pIo = test::ConnectToLocalHost(pPool->service(), test::Local_Host_Port, context.publicKey());

		// Assert: a single connection was accepted
		EXPECT_EQ(1u, context.counter(Counter_Name));
	}

	// endregion

	// region handlers

	namespace {
		struct SquaresTraits {
			using RequestStructureType = uint64_t;
			using SupplierResultsType = std::vector<uint64_t>;

			static constexpr auto Packet_Type = static_cast<ionet::PacketType>(25);

			static auto ToPayload(const SupplierResultsType& results) {
				auto payloadSize = utils::checked_cast<size_t, uint32_t>(results.size() * sizeof(uint64_t));
				auto pPacket = ionet::CreateSharedPacket<ionet::Packet>(payloadSize);
				pPacket->Type = Packet_Type;
				std::memcpy(pPacket->Data(), results.data(), payloadSize);
				return pPacket;
			}
		};

		std::shared_ptr<ionet::Packet> GenerateSquaresPacket() {
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
		using HandlerFactory = handlers::BatchHandlerFactory<SquaresTraits>;
		packetHandlers.registerHandler(HandlerFactory::Packet_Type, HandlerFactory::Create([](const auto& range) {
			std::vector<uint64_t> squares;
			for (const auto& value : range)
				squares.push_back(value * value);

			return squares;
		}));

		context.boot();

		// - connect to the server as a reader
		auto pPool = test::CreateStartedIoServiceThreadPool();
		auto pIo = test::ConnectToLocalHost(pPool->service(), test::Local_Host_Port, context.publicKey());

		// Sanity: a single connection was accepted
		EXPECT_EQ(1u, context.counter(Counter_Name));

		// Act: send a simple squares request
		ionet::ByteBuffer packetBuffer;
		auto pRequestPacket = GenerateSquaresPacket();
		pIo->write(pRequestPacket, [&service = pPool->service(), &io = *pIo, &packetBuffer](auto) {
			test::AsyncReadIntoBuffer(service, io, packetBuffer);
		});

		pPool->join();

		// Assert: the requested squared values should have been returned
		auto pResponsePacket = reinterpret_cast<const ionet::PacketHeader*>(packetBuffer.data());
		ASSERT_TRUE(!!pResponsePacket);
		ASSERT_EQ(sizeof(ionet::PacketHeader) + 3 * sizeof(uint64_t), pResponsePacket->Size);
		EXPECT_EQ(static_cast<ionet::PacketType>(SquaresTraits::Packet_Type), pResponsePacket->Type);

		const auto* pData = reinterpret_cast<const uint64_t*>(pResponsePacket + 1);
		EXPECT_EQ(9u, pData[0]);
		EXPECT_EQ(64u, pData[1]);
		EXPECT_EQ(25u, pData[2]);
	}

	// endregion

	// region tasks

	TEST(TEST_CLASS, AgePeersTaskIsScheduled) {
		// Assert:
		test::AssertRegisteredTask(TestContext(), 1, "age peers task for service Readers");
	}

	// endregion
}}
