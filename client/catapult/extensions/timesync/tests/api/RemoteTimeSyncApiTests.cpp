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

#include "timesync/src/api/RemoteTimeSyncApi.h"
#include "timesync/src/api/TimeSyncPackets.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/test/other/RemoteApiTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace api {

	namespace {
		struct NetworkTimeTraits {
			static auto Invoke(const RemoteTimeSyncApi& api) {
				return api.networkTime();
			}

			static auto CreateValidResponsePacket() {
				auto pResponsePacket = ionet::CreateSharedPacket<NetworkTimePacket>();
				pResponsePacket->CommunicationTimestamps.SendTimestamp = Timestamp(123);
				pResponsePacket->CommunicationTimestamps.ReceiveTimestamp = Timestamp(234);
				return pResponsePacket;
			}

			static auto CreateMalformedResponsePacket() {
				// just change the size because no responses are intrinsically invalid
				auto pResponsePacket = CreateValidResponsePacket();
				--pResponsePacket->Size;
				return pResponsePacket;
			}

			static void ValidateRequest(const ionet::Packet& packet) {
				EXPECT_TRUE(ionet::IsPacketValid(packet, NetworkTimePacket::Packet_Type));
			}

			static void ValidateResponse(const ionet::Packet&, const timesync::CommunicationTimestamps& timestamps) {
				EXPECT_EQ(Timestamp(123), timestamps.SendTimestamp);
				EXPECT_EQ(Timestamp(234), timestamps.ReceiveTimestamp);
			}
		};

		struct RemoteTimeSyncApiTraits {
			static auto Create(ionet::PacketIo& packetIo) {
				return CreateRemoteTimeSyncApi(packetIo);
			}
		};
	}

	DEFINE_REMOTE_API_TESTS_EMPTY_RESPONSE_INVALID(RemoteTimeSyncApi, NetworkTime)
}}
