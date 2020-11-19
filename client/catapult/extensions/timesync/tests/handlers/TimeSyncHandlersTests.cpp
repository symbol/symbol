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

#include "timesync/src/handlers/TimeSyncHandlers.h"
#include "timesync/src/CommunicationTimestamps.h"
#include "tests/test/core/PacketPayloadTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace handlers {

#define TEST_CLASS TimeSyncHandlersTests

	// region RegisterTimeSyncNetworkTimeHandler

	namespace {
		template<typename TAssert>
		void RunNetworkTimeHandlerTest(
				uint32_t packetExtraSize,
				const timesync::CommunicationTimestamps& communicationTimestamps,
				TAssert assertFunc) {
			// Arrange:
			auto count = 0u;
			extensions::ExtensionManager::NetworkTimeSupplier networkTimeSupplier = [&count, &communicationTimestamps]() {
				// handler will use the networkTimeSupplier twice, first for the receive timestamp and then for the send timestamp
				return 0 == count++
						? communicationTimestamps.ReceiveTimestamp
						: communicationTimestamps.SendTimestamp;
			};
			ionet::ServerPacketHandlers handlers;
			RegisterTimeSyncNetworkTimeHandler(handlers, networkTimeSupplier);

			// - create a valid request
			auto pPacket = ionet::CreateSharedPacket<ionet::Packet>();
			pPacket->Type = ionet::PacketType::Time_Sync_Network_Time;
			pPacket->Size += packetExtraSize;

			// Act:
			ionet::ServerPacketHandlerContext handlerContext;
			EXPECT_TRUE(handlers.process(*pPacket, handlerContext));

			// Assert:
			assertFunc(communicationTimestamps, handlerContext);
		}
	}

	TEST(TEST_CLASS, NetworkTimeHandler_DoesNotRespondToMalformedRequest) {
		// Arrange:
		timesync::CommunicationTimestamps communicationTimestamps(Timestamp(234), Timestamp(123));
		RunNetworkTimeHandlerTest(1, { Timestamp(234), Timestamp(123) }, [](const auto&, const auto& handlerContext) {
			// Assert: malformed packet is ignored
			test::AssertNoResponse(handlerContext);
		});
	}

	TEST(TEST_CLASS, NetworkTimeHandler_WritesCommunicationTimestampsInResponseToValidRequest) {
		// Arrange:
		RunNetworkTimeHandlerTest(0, { Timestamp(234), Timestamp(123) }, [](const auto&, const auto& handlerContext) {
			// Assert: communication timestamps are written
			auto dataSize = sizeof(timesync::CommunicationTimestamps);
			test::AssertPacketHeader(handlerContext, sizeof(ionet::Packet) + dataSize, ionet::PacketType::Time_Sync_Network_Time);

			const auto* pResponse = reinterpret_cast<const Timestamp::ValueType*>(test::GetSingleBufferData(handlerContext));
			EXPECT_EQ(234u, pResponse[0]);
			EXPECT_EQ(123u, pResponse[1]);
		});
	}

	// endregion
}}
