#include "catapult/ionet/PacketHandlers.h"
#include "tests/test/core/PacketPayloadTestUtils.h"
#include "tests/TestHarness.h"
#include <memory>

namespace catapult { namespace ionet {

	using PacketHandlers = ServerPacketHandlers;

	namespace {
		void RegisterHandler(PacketHandlers& handlers, uint32_t type) {
			handlers.registerHandler(static_cast<PacketType>(type), [](const auto&, const auto&) {});
		}

		// marker is split up so that each nibble corresponds to a registered handler (the rightmost marker nibble
		// corresponds to the last handler)
		// if a handler is called, its corresponding nibble is set to the 1-based call index (so call order can be
		// verified too)
		void RegisterHandlers(PacketHandlers& handlers, const std::vector<uint32_t>& types, uint32_t& marker) {
			if (types.size() > sizeof(marker) * 2)
				CATAPULT_THROW_RUNTIME_ERROR_1("marker is too small for requested number of handlers", marker);

			// Act:
			auto pCounter = std::make_shared<uint32_t>(1);
			uint32_t i = 0;
			for (auto type : types) {
				auto bits = (types.size() - i - 1) * 4u;
				handlers.registerHandler(static_cast<PacketType>(type), [bits, pCounter, &marker](const auto&, const auto&) {
					marker |= *pCounter << bits;
					++*pCounter;
				});
				++i;
			}
		}

		size_t ProcessPacket(PacketHandlers& handlers, uint32_t type) {
			// Arrange:
			Packet packet;
			packet.Type = static_cast<PacketType>(type);

			// Act:
			ServerPacketHandlerContext context;
			return handlers.process(packet, context);
		}
	}

	// region ServerPacketHandlerContext

	TEST(PacketHandlersTests, ServerPacketHandlerContextInitiallyHasNoResponse) {
		// Act:
		ServerPacketHandlerContext context;

		// Assert:
		EXPECT_FALSE(context.hasResponse());
	}

	TEST(PacketHandlersTests, ServerPacketHandlerCannotAccessUnsetResponse) {
		// Act:
		ServerPacketHandlerContext context;

		// Assert:
		EXPECT_THROW(context.response(), catapult_runtime_error);
	}

	TEST(PacketHandlersTests, ServerPacketHandlerContextCanHaveResponseSet) {
		// Arrange:
		auto pPacket = CreateSharedPacket<Packet>(25);
		ServerPacketHandlerContext context;

		// Act:
		context.response(pPacket);

		// Assert:
		EXPECT_TRUE(context.hasResponse());
		test::AssertPacketPayload(*pPacket, context.response());
	}

	TEST(PacketHandlersTests, ServerPacketHandlerContextCannotHaveMultipleResponsesSet) {
		// Arrange:
		auto pPacket = CreateSharedPacket<Packet>(25);
		ServerPacketHandlerContext context;
		context.response(pPacket);

		// Act:
		EXPECT_THROW(context.response(pPacket), catapult_runtime_error);
	}

	// endregion

	TEST(PacketHandlersTests, HandlersAreInitiallyEmpty) {
		// Act:
		PacketHandlers handlers;

		// Assert:
		EXPECT_EQ(0u, handlers.size());
	}

	TEST(PacketHandlersTests, CanAddSingleHandler) {
		// Act:
		PacketHandlers handlers;
		RegisterHandler(handlers, 1);

		// Assert:
		EXPECT_EQ(1u, handlers.size());
	}

	TEST(PacketHandlersTests, CanAddMultipleHandlers) {
		// Act:
		auto marker = 0u;
		PacketHandlers handlers;
		RegisterHandlers(handlers, { 1, 2, 7 }, marker);

		// Assert:
		EXPECT_EQ(3u, handlers.size());
	}

	TEST(PacketHandlersTests, CannotAddMultipleHandlersForSamePacketType) {
		// Arrange:
		auto marker = 0u;
		PacketHandlers handlers;
		RegisterHandlers(handlers, { 1, 2 }, marker);

		// Assert:
		EXPECT_THROW(RegisterHandler(handlers, 1), catapult_runtime_error);
	}

	TEST(PacketHandlersTests, CanProcessReturnsTrueForPacketWithRegisteredHandler) {
		// Arrange:
		uint32_t marker = 0u;
		PacketHandlers handlers;
		RegisterHandlers(handlers, { 1, 3, 5 }, marker);

		for (auto type : { 1, 3, 5 }) {
			// Act:
			Packet packet;
			packet.Type = static_cast<PacketType>(type);
			auto canProcess = handlers.canProcess(packet);

			// Assert:
			EXPECT_TRUE(canProcess) << "type " << type;
		}
	}

	TEST(PacketHandlersTests, CanProcessReturnsFalseForPacketWithNoRegisteredHandler) {
		// Arrange:
		uint32_t marker = 0u;
		PacketHandlers handlers;
		RegisterHandlers(handlers, { 1, 3, 5 }, marker);

		for (auto type : { 0, 2, 4, 7 }) {
			// Act:
			Packet packet;
			packet.Type = static_cast<PacketType>(type);
			auto canProcess = handlers.canProcess(packet);

			// Assert:
			EXPECT_FALSE(canProcess) << "type " << type;
		}
	}

	TEST(PacketHandlersTests, PacketHandlerForZeroTypeIsNotInitiallyRegistered) {
		// Act:
		Packet packet;
		packet.Type = static_cast<PacketType>(0);
		PacketHandlers handlers;

		// Assert:
		EXPECT_EQ(0u, handlers.size());
		EXPECT_FALSE(handlers.canProcess(packet));
	}

	TEST(PacketHandlersTests, CanAddHandlerForZeroType) {
		// Arrange:
		Packet packet;
		packet.Type = static_cast<PacketType>(0);
		PacketHandlers handlers;

		// Act:
		RegisterHandler(handlers, 0);

		// Assert:
		EXPECT_EQ(1u, handlers.size());
		EXPECT_TRUE(handlers.canProcess(packet));
	}

	namespace {
		void AssertNoMatchingHandlers(uint32_t type) {
			// Arrange:
			uint32_t marker = 0u;
			PacketHandlers handlers;
			RegisterHandlers(handlers, { 1, 3, 5 }, marker);

			// Act:
			auto isProcessed = ProcessPacket(handlers, type);

			// Assert:
			EXPECT_FALSE(isProcessed);
			EXPECT_EQ(0x00000000u, marker);
		}
	}

	TEST(PacketHandlersTests, CanProcessPacketWithZeroMatchingHandlers) {
		// Assert:
		AssertNoMatchingHandlers(4); // empty slot
		AssertNoMatchingHandlers(44); // beyond end
	}

	TEST(PacketHandlersTests, CanProcessPacketWithSingleMatchingHandler) {
		// Arrange:
		auto marker = 0u;
		PacketHandlers handlers;
		RegisterHandlers(handlers, { 1, 3, 5 }, marker);

		// Act:
		auto isProcessed = ProcessPacket(handlers, 3);

		// Assert:
		EXPECT_TRUE(isProcessed);
		EXPECT_EQ(0x00000010u, marker);
	}

	TEST(PacketHandlersTests, PacketIsPassedToHandler) {
		// Arrange:
		static const uint32_t Packet_Size = 25;
		PacketHandlers handlers;
		ByteBuffer packetBufferPassedToHandler(Packet_Size);
		handlers.registerHandler(static_cast<PacketType>(1), [&packetBufferPassedToHandler](const auto& packet, const auto&) {
			std::memcpy(&packetBufferPassedToHandler[0], &packet, Packet_Size);
		});

		// Act:
		auto packetBuffer = test::GenerateRandomVector(Packet_Size);
		auto pPacket = reinterpret_cast<Packet*>(&packetBuffer[0]);
		pPacket->Size = Packet_Size;
		pPacket->Type = static_cast<PacketType>(1);
		ServerPacketHandlerContext handlerContext;
		handlers.process(*pPacket, handlerContext);

		// Assert:
		EXPECT_EQ(packetBuffer, packetBufferPassedToHandler);
	}

	TEST(PacketHandlersTests, MatchingHandlerCanRespondViaContext) {
		// Arrange: set up 3 handlers for packet types 1, 2, 3
		//          the handler will write the packet type to the response packet based on the matching type
		static const uint32_t Num_Handlers = 3;
		PacketHandlers handlers;
		auto numCallbackCalls = 0u;
		for (auto i = 0u; i < Num_Handlers; ++i) {
			handlers.registerHandler(static_cast<PacketType>(i), [i, &numCallbackCalls](const auto&, auto& context) {
				auto pResponsePacket = CreateSharedPacket<Packet>();
				pResponsePacket->Type = static_cast<PacketType>(0xFF ^ (1 << i));
				context.response(pResponsePacket);
				++numCallbackCalls;
			});
		}

		// Act:
		Packet packet;
		packet.Size = sizeof(Packet);
		packet.Type = static_cast<PacketType>(2);
		ServerPacketHandlerContext handlerContext;
		handlers.process(packet, handlerContext);

		// Assert:
		EXPECT_EQ(1u, numCallbackCalls);
		EXPECT_EQ(static_cast<PacketType>(0xFB), handlerContext.response().header().Type);
	}
}}
