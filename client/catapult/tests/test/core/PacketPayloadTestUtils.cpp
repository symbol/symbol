#include "PacketPayloadTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	void AssertPacketPayload(const ionet::Packet& packet, const ionet::PacketPayload& payload) {
		// Assert: payload is a thin wrapper around packet
		AssertPacketHeader(payload, packet.Size, packet.Type);

		const auto& buffers = payload.buffers();
		ASSERT_EQ(1u, buffers.size());

		const auto& buffer = buffers[0];
		EXPECT_EQ(packet.Data(), buffer.pData);
		EXPECT_EQ(packet.Size - sizeof(ionet::PacketHeader), buffer.Size);
	}

	void AssertPacketHeader(const ionet::PacketPayload& payload, size_t expectedSize, ionet::PacketType expectedType) {
		// Assert: the payload is set and it has the expected size and type
		EXPECT_FALSE(payload.unset());

		const auto& header = payload.header();
		EXPECT_EQ(expectedSize, header.Size);
		EXPECT_EQ(expectedType, header.Type);
	}

	void AssertNoResponse(const ionet::ServerPacketHandlerContext& context) {
		// Assert: the context does not have a response
		EXPECT_FALSE(context.hasResponse());
	}

	void AssertPacketHeader(
			const ionet::ServerPacketHandlerContext& context,
			size_t expectedSize,
			ionet::PacketType expectedType) {
		// Assert: the context has a response with a header that has the expected size and type
		ASSERT_TRUE(context.hasResponse());
		AssertPacketHeader(context.response(), expectedSize, expectedType);
	}

	const uint8_t* GetSingleBufferData(const ionet::ServerPacketHandlerContext& context) {
		// Assert: context has a response with a single buffer
		const auto& buffers = context.response().buffers();
		if (1u != buffers.size())
			CATAPULT_THROW_RUNTIME_ERROR_1("response has unexpected number of buffers", buffers.size());

		return buffers[0].pData;
	}
}}
