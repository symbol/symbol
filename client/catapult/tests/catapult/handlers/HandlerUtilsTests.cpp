#include "catapult/handlers/HandlerUtils.h"
#include "catapult/model/Block.h"
#include "catapult/model/TransactionPlugin.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

	// region CreatePushEntityHandler

	namespace {
		constexpr auto Block_Packet_Size = sizeof(ionet::PacketHeader) + sizeof(model::Block);
		constexpr auto Two_Blocks_Packet_Size = sizeof(ionet::PacketHeader) + 2 * sizeof(model::Block);

		void AssertCreatePushEntityHandlerForwarding(const ionet::Packet& packet, size_t numExpectedForwards) {
			// Arrange:
			auto counter = 0u;
			model::TransactionRegistry registry;
			auto handler = CreatePushEntityHandler<model::Block>(registry, [&counter](const auto&) { ++counter; });

			// Act:
			handler(packet, ionet::ServerPacketHandlerContext());

			// Assert:
			EXPECT_EQ(numExpectedForwards, counter);
		}
	}

	TEST(HandlerUtilsTests, CreatePushEntityHandler_DoesNotForwardMalformedEntityToRangeHandler) {
		// Arrange:
		ionet::ByteBuffer buffer(Block_Packet_Size);
		auto& packet = test::SetPushBlockPacketInBuffer(buffer);
		--packet.Size;

		// Assert:
		AssertCreatePushEntityHandlerForwarding(packet, 0);
	}

	TEST(HandlerUtilsTests, CreatePushEntityHandler_ForwardsWellFormedEntityToRangeHandler) {
		// Arrange:
		ionet::ByteBuffer buffer(Block_Packet_Size);
		const auto& packet = test::SetPushBlockPacketInBuffer(buffer);

		// Assert:
		AssertCreatePushEntityHandlerForwarding(packet, 1);
	}

	TEST(HandlerUtilsTests, CreatePushEntityHandler_DoesNotForwardMalformedEntitiesToRangeHandler) {
		// Arrange:
		ionet::ByteBuffer buffer(Two_Blocks_Packet_Size);
		auto& packet = test::SetPushBlockPacketInBuffer(buffer);
		test::SetBlockAt(buffer, sizeof(ionet::Packet));
		test::SetBlockAt(buffer, sizeof(ionet::Packet) + sizeof(model::Block));
		--packet.Size;

		// Assert:
		AssertCreatePushEntityHandlerForwarding(packet, 0);
	}

	TEST(HandlerUtilsTests, CreatePushEntityHandler_ForwardsWellFormedEntitiesToRangeHandler) {
		// Arrange:
		ionet::ByteBuffer buffer(Two_Blocks_Packet_Size);
		const auto& packet = test::SetPushBlockPacketInBuffer(buffer);
		test::SetBlockAt(buffer, sizeof(ionet::Packet));
		test::SetBlockAt(buffer, sizeof(ionet::Packet) + sizeof(model::Block));

		// Assert:
		AssertCreatePushEntityHandlerForwarding(packet, 1);
	}

	// endregion
}}
