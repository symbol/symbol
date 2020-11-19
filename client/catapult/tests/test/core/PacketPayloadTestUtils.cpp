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
		ASSERT_EQ(packet.Size - sizeof(ionet::PacketHeader), buffer.Size);
	}

	void AssertPacketPayloadUnset(const ionet::PacketPayload& payload) {
		// Assert: the payload is unset
		EXPECT_TRUE(payload.unset());

		const auto& header = payload.header();
		EXPECT_EQ(0u, header.Size);
		EXPECT_EQ(ionet::PacketType::Undefined, header.Type);

		EXPECT_TRUE(payload.buffers().empty());
	}

	void AssertPacketHeader(const ionet::PacketPayload& payload, size_t expectedSize, ionet::PacketType expectedType) {
		// Assert: the payload is set and it has the expected size and type
		EXPECT_FALSE(payload.unset());

		const auto& header = payload.header();
		EXPECT_EQ(expectedSize, header.Size);
		EXPECT_EQ(expectedType, header.Type);
	}

	void AssertNoResponse(const ionet::ServerPacketHandlerContext& handlerContext) {
		// Assert: handlerContext does not have a response
		EXPECT_FALSE(handlerContext.hasResponse());
	}

	void AssertPacketHeader(const ionet::ServerPacketHandlerContext& handlerContext, size_t expectedSize, ionet::PacketType expectedType) {
		// Assert: handlerContext has a response with a header that has the expected size and type
		ASSERT_TRUE(handlerContext.hasResponse());
		AssertPacketHeader(handlerContext.response(), expectedSize, expectedType);
	}

	const uint8_t* GetSingleBufferData(const ionet::ServerPacketHandlerContext& handlerContext) {
		// Assert: handlerContext has a response with a single buffer
		const auto& buffers = handlerContext.response().buffers();
		if (1u != buffers.size())
			CATAPULT_THROW_RUNTIME_ERROR_1("response has unexpected number of buffers", buffers.size());

		return buffers[0].pData;
	}

	void AssertEqualPayload(const ionet::PacketPayload& expectedPayload, const ionet::PacketPayload& payload) {
		auto expectedHeader = expectedPayload.header();
		auto header = payload.header();
		EXPECT_EQ(expectedHeader.Size, header.Size);
		EXPECT_EQ(expectedHeader.Type, header.Type);

		auto expectedBuffers = expectedPayload.buffers();
		auto buffers = payload.buffers();
		ASSERT_EQ(expectedBuffers.size(), buffers.size());
		for (auto i = 0u; i < expectedBuffers.size(); ++i) {
			auto message = "at index " + std::to_string(i);
			ASSERT_EQ(expectedBuffers[i].Size, buffers[i].Size) << message;
			EXPECT_EQ_MEMORY(expectedBuffers[i].pData, buffers[i].pData, expectedBuffers[i].Size) << message;
		}
	}
}}
