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
#include "catapult/config/NodeConfiguration.h"
#include "catapult/ionet/PacketHandlers.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Handlers trusted host test suite.
	template<typename TTestContext>
	struct HandlersTrustedHostTests {
	public:
		static void AssertHandlersAreOnlyAccessibleToTrustedHosts(ionet::PacketType packetType) {
			// Arrange:
			TTestContext context;
			const_cast<config::NodeConfiguration&>(context.testState().state().config().Node).TrustedHosts = { "foo.bar" };

			// Act:
			context.boot();
			const auto& packetHandlers = context.testState().state().packetHandlers();

			// Assert: service-registered handler has custom host filtering
			EXPECT_TRUE(CanProcessPacketFromHost(packetHandlers, packetType, "foo.bar"));
			EXPECT_FALSE(CanProcessPacketFromHost(packetHandlers, packetType, "foo.baz"));
		}

		static void AssertTrustedHostsFilterIsClearedAfterAllHandlersAreRegistered() {
			// Arrange:
			TTestContext context;
			const_cast<config::NodeConfiguration&>(context.testState().state().config().Node).TrustedHosts = { "foo.bar" };

			// Act:
			context.boot();
			const auto& packetHandlers = context.testState().state().packetHandlers();

			// - register a new handler after the service was booted
			constexpr auto Zero_Packet_Type = static_cast<ionet::PacketType>(0);
			const_cast<ionet::ServerPacketHandlers&>(packetHandlers).registerHandler(Zero_Packet_Type, [](const auto&, const auto&) {});

			// Assert: external-registered handler has no custom host filtering
			EXPECT_TRUE(CanProcessPacketFromHost(packetHandlers, Zero_Packet_Type, "foo.bar"));
			EXPECT_TRUE(CanProcessPacketFromHost(packetHandlers, Zero_Packet_Type, "foo.baz"));
		}

	private:
		static bool CanProcessPacketFromHost(
				const ionet::ServerPacketHandlers& packetHandlers,
				ionet::PacketType packetType,
				const std::string& hostname) {
			// prepare a packet
			auto pPacket = ionet::CreateSharedPacket<ionet::Packet>();
			pPacket->Type = packetType;

			// attempt to process
			ionet::ServerPacketHandlerContext handlerContext({}, hostname);
			return packetHandlers.process(*pPacket, handlerContext);
		}
	};

#define ADD_HANDLERS_TRUSTED_HOSTS_TESTS(TEST_CONTEXT, PACKET_TYPE) \
	TEST(TEST_CLASS, HandlersAreOnlyAccessibleToTrustedHosts) { \
		test::HandlersTrustedHostTests<TEST_CONTEXT>::AssertHandlersAreOnlyAccessibleToTrustedHosts(PACKET_TYPE); \
	} \
	TEST(TEST_CLASS, TrustedHostsFilterIsClearedAfterAllHandlersAreRegistered) { \
		test::HandlersTrustedHostTests<TEST_CONTEXT>::AssertTrustedHostsFilterIsClearedAfterAllHandlersAreRegistered(); \
	}
}}
