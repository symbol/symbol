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

#include "diagnostics/src/DiagnosticsService.h"
#include "catapult/model/DiagnosticCounterValue.h"
#include "tests/test/core/HandlersTrustedHostTests.h"
#include "tests/test/core/PacketPayloadTestUtils.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace diagnostics {

#define TEST_CLASS DiagnosticsServiceTests

	namespace {
		struct DiagnosticsServiceTraits {
			static constexpr auto CreateRegistrar = CreateDiagnosticsServiceRegistrar;
		};

		using TestContext = test::ServiceLocatorTestContext<DiagnosticsServiceTraits>;
	}

	ADD_SERVICE_REGISTRAR_INFO_TEST(Diagnostics, Initial)

	TEST(TEST_CLASS, NoServicesOrCountersAreRegistered) {
		test::AssertNoServicesOrCountersAreRegistered<TestContext>();
	}

	TEST(TEST_CLASS, TasksAreRegistered) {
		test::AssertRegisteredTasks(TestContext(), { "logging task" });
	}

	TEST(TEST_CLASS, PacketHandlersAreRegistered) {
		// Arrange:
		struct HookCapture {
			const ionet::ServerPacketHandlers* pHandlers;
			const cache::CatapultCache* pCache;
		};

		HookCapture capture;
		TestContext context;
		context.testState().pluginManager().addDiagnosticHandlerHook([&capture](auto& handlers, const auto& cache) {
			// - capture params and register a handler
			capture.pHandlers = &handlers;
			capture.pCache = &cache;
			handlers.registerHandler(ionet::PacketType::Chain_Statistics, [](const auto&, const auto&) {});
		});

		// Act:
		context.boot();
		const auto& packetHandlers = context.testState().state().packetHandlers();

		// Assert: three default handlers were added
		EXPECT_EQ(4u, packetHandlers.size());
		EXPECT_TRUE(packetHandlers.canProcess(ionet::PacketType::Diagnostic_Counters)); // the default (counters) diagnostic handler
		EXPECT_TRUE(packetHandlers.canProcess(ionet::PacketType::Active_Node_Infos)); // the default (nodes) diagnostic handler
		EXPECT_TRUE(packetHandlers.canProcess(ionet::PacketType::Block_Statement)); // the default (statements) diagnostic handler
		EXPECT_TRUE(packetHandlers.canProcess(ionet::PacketType::Chain_Statistics)); // the diagnostic handler hook registered above

		// - correct params were forwarded to callback
		EXPECT_EQ(&packetHandlers, capture.pHandlers);
		EXPECT_EQ(&context.testState().state().cache(), capture.pCache);
	}

	ADD_HANDLERS_TRUSTED_HOSTS_TESTS(TestContext, ionet::PacketType::Diagnostic_Counters)

	TEST(TEST_CLASS, CountersAreSourcedFromLocatorAndState) {
		// Arrange: add counters to different sources
		constexpr auto Num_Counters = 2u;
		TestContext context;
		context.locator().registerServiceCounter<uint32_t>("A SERVICE", "ALPHA", [](const auto&) { return 0u; });
		context.testState().counters().push_back(utils::DiagnosticCounter(utils::DiagnosticCounterId("BETA"), []() { return 1u; }));

		// Act:
		context.boot();
		const auto& packetHandlers = context.testState().state().packetHandlers();

		// - process a counters request
		auto pPacket = ionet::CreateSharedPacket<ionet::Packet>();
		pPacket->Type = ionet::PacketType::Diagnostic_Counters;
		ionet::ServerPacketHandlerContext handlerContext;
		EXPECT_TRUE(packetHandlers.process(*pPacket, handlerContext));

		// Assert: header is correct and contains the expected number of counters
		auto expectedPacketSize = sizeof(ionet::PacketHeader) + Num_Counters * sizeof(model::DiagnosticCounterValue);
		test::AssertPacketHeader(handlerContext, expectedPacketSize, ionet::PacketType::Diagnostic_Counters);

		// - check the counter names
		std::set<std::string> actualCounterNames;
		const auto* pCounterValue = reinterpret_cast<const model::DiagnosticCounterValue*>(test::GetSingleBufferData(handlerContext));
		for (auto i = 0u; i < Num_Counters; ++i) {
			actualCounterNames.insert(utils::DiagnosticCounterId(pCounterValue->Id).name());
			++pCounterValue;
		}

		EXPECT_EQ(Num_Counters, actualCounterNames.size());
		EXPECT_EQ(std::set<std::string>({ "ALPHA", "BETA" }), actualCounterNames);
	}
}}
