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

#pragma once
#include "PacketIoTestUtils.h"
#include "mocks/MockPacketSocket.h"

namespace catapult { namespace test {

	/// Packet socket decorator test suite.
	template<typename TTraits>
	struct PacketSocketDecoratorTests {
	public:
		static void AssertCanRoundtripPackets() {
			// Arrange:
			typename TTraits::TestContextType context;

			// Act + Assert:
			test::AssertCanRoundtripPackets(*context.pMockPacketSocket, *context.pDecoratedSocket);
		}

		static void AssertCanRoundtripPacketsWithReadMultiple() {
			// Arrange:
			typename TTraits::TestContextType context;

			// Act + Assert:
			test::AssertCanRoundtripPackets(*context.pMockPacketSocket, *context.pDecoratedSocket, *context.pDecoratedSocket);
		}

		static void AssertCanRoundtripBufferedPackets() {
			// Arrange:
			typename TTraits::TestContextType context;
			context.pMockPacketSocket->enableBufferedIoRoundtrip();

			// Act + Assert:
			test::AssertCanRoundtripPackets(*context.pMockPacketSocket, *context.pDecoratedSocket->buffered());

			// Sanity:
			EXPECT_EQ(1u, context.pMockPacketSocket->numBufferedCalls());
		}

		static void AssertCanAccessStats() {
			// Arrange:
			typename TTraits::TestContextType context;

			// Act:
			ionet::PacketSocket::Stats capturedStats;
			context.pDecoratedSocket->stats([&capturedStats](const auto& stats) {
				capturedStats = stats;
			});

			// Assert: NumUnprocessedBytes is set to call count by MockPacketSocket
			EXPECT_EQ(1u, context.pMockPacketSocket->numStatsCalls());
			EXPECT_EQ(1u, capturedStats.NumUnprocessedBytes);
		}

		static void AssertCanWaitForData() {
			// Arrange:
			typename TTraits::TestContextType context;

			// Act:
			auto numCallbackCalls = 0u;
			context.pDecoratedSocket->waitForData([&numCallbackCalls]() {
				++numCallbackCalls;
			});

			// Assert:
			EXPECT_EQ(1u, numCallbackCalls);
		}

		static void AssertCanClose() {
			// Arrange:
			typename TTraits::TestContextType context;

			// Act:
			context.pDecoratedSocket->close();

			// Assert:
			EXPECT_EQ(1u, context.pMockPacketSocket->numCloseCalls());
		}

		static void AssertCanAbort() {
			// Arrange:
			typename TTraits::TestContextType context;

			// Act:
			context.pDecoratedSocket->abort();

			// Assert:
			EXPECT_EQ(1u, context.pMockPacketSocket->numAbortCalls());
		}
	};
}}

#define MAKE_PACKET_SOCKET_DECORATOR_TEST(TRAITS_NAME, PREFIX, TEST_NAME) \
	TEST(TEST_CLASS, PREFIX##TEST_NAME) { test::PacketSocketDecoratorTests<TRAITS_NAME>::Assert##TEST_NAME(); }

#define DEFINE_PACKET_SOCKET_DECORATOR_TESTS(TRAITS_NAME, PREFIX) \
	MAKE_PACKET_SOCKET_DECORATOR_TEST(TRAITS_NAME, PREFIX, CanRoundtripPackets) \
	MAKE_PACKET_SOCKET_DECORATOR_TEST(TRAITS_NAME, PREFIX, CanRoundtripPacketsWithReadMultiple) \
	MAKE_PACKET_SOCKET_DECORATOR_TEST(TRAITS_NAME, PREFIX, CanRoundtripBufferedPackets) \
	MAKE_PACKET_SOCKET_DECORATOR_TEST(TRAITS_NAME, PREFIX, CanAccessStats) \
	MAKE_PACKET_SOCKET_DECORATOR_TEST(TRAITS_NAME, PREFIX, CanWaitForData) \
	MAKE_PACKET_SOCKET_DECORATOR_TEST(TRAITS_NAME, PREFIX, CanClose) \
	MAKE_PACKET_SOCKET_DECORATOR_TEST(TRAITS_NAME, PREFIX, CanAbort)
