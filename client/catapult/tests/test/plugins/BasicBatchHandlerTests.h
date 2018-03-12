#pragma once
#include "tests/test/core/PacketPayloadTestUtils.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// A container of basic batch handler tests.
	template<typename TTraits>
	struct BasicBatchHandlerTests {
	public:
		/// Asserts that a packet with a reported size less than a packet header is rejected.
		static void AssertTooSmallPacketIsRejected() {
			// Arrange:
			ionet::Packet packet;
			packet.Size = sizeof(ionet::PacketHeader) - 1;
			packet.Type = TTraits::Packet_Type;

			// Assert:
			AssertPacketIsRejected(packet);
		}

		/// Asserts that a packet with the wrong type is rejected.
		static void AssertPacketWithWrongTypeIsRejected() {
			// Assert: wrong packet type
			AssertPacketIsRejected(TTraits::Valid_Request_Payload_Size, ionet::PacketType::Chain_Info, false);
		}

		/// Asserts that a packet with an invalid payload size is rejected.
		static void AssertPacketWithInvalidPayloadIsRejected() {
			// Assert: payload size is not divisible by TTraits::ValidPayloadSize
			AssertPacketIsRejected(TTraits::Valid_Request_Payload_Size + TTraits::Valid_Request_Payload_Size / 2, TTraits::Packet_Type);
		}

	private:
		static void RegisterHandler(ionet::ServerPacketHandlers& handlers, size_t& counter) {
			TTraits::Register_Handler_Func(handlers, [&](const auto&) {
				++counter;
				return typename TTraits::ResponseType();
			});
		}

		static void AssertPacketIsRejected(const ionet::Packet& packet, bool expectedCanProcessPacketType = true) {
			// Arrange:
			size_t counter = 0;
			ionet::ServerPacketHandlers handlers;
			RegisterHandler(handlers, counter);

			// Act:
			ionet::ServerPacketHandlerContext context({}, "");
			EXPECT_EQ(expectedCanProcessPacketType, handlers.process(packet, context));

			// Assert:
			EXPECT_EQ(0u, counter);
			test::AssertNoResponse(context);
		}

	protected:
		static void AssertPacketIsRejected(uint32_t payloadSize, ionet::PacketType type, bool expectedCanProcessPacketType = true) {
			// Arrange:
			auto pPacket = test::CreateRandomPacket(payloadSize, type);

			// Assert:
			AssertPacketIsRejected(*pPacket, expectedCanProcessPacketType);
		}

		static void AssertValidPacketWithElementsIsAccepted(uint32_t numElements) {
			// Arrange:
			size_t counter = 0;
			ionet::ServerPacketHandlers handlers;
			RegisterHandler(handlers, counter);

			auto pPacket = test::CreateRandomPacket(numElements * TTraits::Valid_Request_Payload_Size, TTraits::Packet_Type);

			// Act:
			ionet::ServerPacketHandlerContext context({}, "");
			EXPECT_TRUE(handlers.process(*pPacket, context));

			// Assert:
			EXPECT_EQ(1u, counter);
			ASSERT_TRUE(context.hasResponse());
			test::AssertPacketHeader(context, sizeof(ionet::PacketHeader), TTraits::Packet_Type);
			EXPECT_TRUE(context.response().buffers().empty());
		}
	};
}}
