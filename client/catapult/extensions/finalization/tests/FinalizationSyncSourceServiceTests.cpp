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

#include "finalization/src/FinalizationSyncSourceService.h"
#include "finalization/src/FinalizationBootstrapperService.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace finalization {

#define TEST_CLASS FinalizationSyncSourceServiceTests

	namespace {
		struct FinalizationSyncSourceServiceTraits {
			static constexpr auto CreateRegistrar = CreateFinalizationSyncSourceServiceRegistrar;
		};

		class TestContext : public test::ServiceLocatorTestContext<FinalizationSyncSourceServiceTraits> {
		public:
			TestContext() {
				// Arrange: register service dependencies
				auto pBootstrapperRegistrar = CreateFinalizationBootstrapperServiceRegistrar();
				pBootstrapperRegistrar->registerServices(locator(), testState().state());

				// - register hook dependencies
				auto& hooks = GetFinalizationServerHooks(locator());
				hooks.setMessageRangeConsumer([](auto&&) {});
			}
		};
	}

	// region basic

	ADD_SERVICE_REGISTRAR_INFO_TEST(FinalizationSyncSource, Post_Extended_Range_Consumers)

	TEST(TEST_CLASS, NoServicesOrCountersAreRegistered) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot();

		// Assert: only dependency services are registered
		EXPECT_EQ(1u, context.locator().numServices());
		EXPECT_EQ(0u, context.locator().counters().size());
	}

	TEST(TEST_CLASS, PacketHandlersAreRegistered) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot();
		const auto& handlers = context.testState().state().packetHandlers();

		// Assert:
		EXPECT_EQ(1u, handlers.size());
		EXPECT_TRUE(handlers.canProcess(ionet::PacketType::Push_Finalization_Messages));
	}

	// endregion
}}
