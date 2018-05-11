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

#include "catapult/handlers/HandlerUtils.h"
#include "catapult/model/Block.h"
#include "catapult/model/TransactionPlugin.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

#define TEST_CLASS HandlerUtilsTests

	// region CreatePushEntityHandler

	namespace {
		constexpr auto Block_Packet_Size = sizeof(ionet::PacketHeader) + sizeof(model::Block);
		constexpr auto Two_Blocks_Packet_Size = sizeof(ionet::PacketHeader) + 2 * sizeof(model::Block);

		void AssertCreatePushEntityHandlerForwarding(const ionet::Packet& packet, size_t numExpectedForwards) {
			// Arrange:
			model::TransactionRegistry registry;
			Key capturedSourcePublicKey;
			auto counter = 0u;
			auto handler = CreatePushEntityHandler<model::Block>(registry, [&capturedSourcePublicKey, &counter](const auto& range) {
				capturedSourcePublicKey = range.SourcePublicKey;
				++counter;
			});

			// Act:
			auto sourcePublicKey = test::GenerateRandomData<Key_Size>();
			handler(packet, ionet::ServerPacketHandlerContext(sourcePublicKey, ""));

			// Assert:
			EXPECT_EQ(numExpectedForwards, counter);

			// - if the callback was called, context should have been forwarded along with the range
			if (numExpectedForwards > 0)
				EXPECT_EQ(sourcePublicKey, capturedSourcePublicKey);
		}
	}

	TEST(TEST_CLASS, CreatePushEntityHandler_DoesNotForwardMalformedEntityToRangeHandler) {
		// Arrange:
		ionet::ByteBuffer buffer(Block_Packet_Size);
		auto& packet = test::SetPushBlockPacketInBuffer(buffer);
		--packet.Size;

		// Assert:
		AssertCreatePushEntityHandlerForwarding(packet, 0);
	}

	TEST(TEST_CLASS, CreatePushEntityHandler_ForwardsWellFormedEntityToRangeHandler) {
		// Arrange:
		ionet::ByteBuffer buffer(Block_Packet_Size);
		const auto& packet = test::SetPushBlockPacketInBuffer(buffer);

		// Assert:
		AssertCreatePushEntityHandlerForwarding(packet, 1);
	}

	TEST(TEST_CLASS, CreatePushEntityHandler_DoesNotForwardMalformedEntitiesToRangeHandler) {
		// Arrange:
		ionet::ByteBuffer buffer(Two_Blocks_Packet_Size);
		auto& packet = test::SetPushBlockPacketInBuffer(buffer);
		test::SetBlockAt(buffer, sizeof(ionet::Packet));
		test::SetBlockAt(buffer, sizeof(ionet::Packet) + sizeof(model::Block));
		--packet.Size;

		// Assert:
		AssertCreatePushEntityHandlerForwarding(packet, 0);
	}

	TEST(TEST_CLASS, CreatePushEntityHandler_ForwardsWellFormedEntitiesToRangeHandler) {
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
