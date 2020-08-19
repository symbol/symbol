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

#include "pluginhandlers/src/PluginHandlersService.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace pluginhandlers {

#define TEST_CLASS PluginHandlersServiceTests

	namespace {
		struct PluginHandlersServiceTraits {
			static constexpr auto CreateRegistrar = CreatePluginHandlersServiceRegistrar;
		};

		using TestContext = test::ServiceLocatorTestContext<PluginHandlersServiceTraits>;
	}

	ADD_SERVICE_REGISTRAR_INFO_TEST(PluginHandlers, Initial)

	TEST(TEST_CLASS, NoServicesOrCountersAreRegistered) {
		test::AssertNoServicesOrCountersAreRegistered<TestContext>();
	}

	TEST(TEST_CLASS, PacketHandlersAreRegistered) {
		// Arrange:
		struct HookCapture {
			const ionet::ServerPacketHandlers* pHandlers;
			const cache::CatapultCache* pCache;
		};

		HookCapture capture;
		TestContext context;
		context.testState().pluginManager().addHandlerHook([&capture](auto& handlers, const auto& cache) {
			// - capture params and register a handler
			capture.pHandlers = &handlers;
			capture.pCache = &cache;
			handlers.registerHandler(ionet::PacketType::Chain_Statistics, [](const auto&, const auto&) {});
		});

		// Act:
		context.boot();
		const auto& packetHandlers = context.testState().state().packetHandlers();

		// Assert:
		EXPECT_EQ(1u, packetHandlers.size());
		EXPECT_TRUE(packetHandlers.canProcess(ionet::PacketType::Chain_Statistics)); // the handler hook registered above

		// - correct params were forwarded to callback
		EXPECT_EQ(&packetHandlers, capture.pHandlers);
		EXPECT_EQ(&context.testState().state().cache(), capture.pCache);
	}
}}
