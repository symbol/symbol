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
#include "finalization/tests/test/FinalizationBootstrapperServiceTestUtils.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace finalization {

#define TEST_CLASS FinalizationSyncSourceServiceTests

	namespace {
		struct FinalizationSyncSourceServiceTraits {
			static auto CreateRegistrar(bool enableVoting) {
				return CreateFinalizationSyncSourceServiceRegistrar(enableVoting);
			}

			static auto CreateRegistrar() {
				return CreateRegistrar(true);
			}
		};

		using TestContext = test::MessageRangeConsumerDependentServiceLocatorTestContext<FinalizationSyncSourceServiceTraits>;
	}

	// region basic

	ADD_SERVICE_REGISTRAR_INFO_TEST(FinalizationSyncSource, Post_Extended_Range_Consumers)

	TEST(TEST_CLASS, NoServicesOrCountersAreRegistered) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot();

		// Assert: only dependency services are registered
		EXPECT_EQ(test::FinalizationBootstrapperServiceTestUtils::Num_Bootstrapper_Services, context.locator().numServices());
		EXPECT_EQ(0u, context.locator().counters().size());
	}

	TEST(TEST_CLASS, PacketHandlersAreRegistered_WithVotingEnabled) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot(true);
		const auto& handlers = context.testState().state().packetHandlers();

		// Assert:
		EXPECT_EQ(5u, handlers.size());
		EXPECT_TRUE(handlers.canProcess(ionet::PacketType::Finalization_Statistics));
		EXPECT_TRUE(handlers.canProcess(ionet::PacketType::Finalization_Proof_At_Point));
		EXPECT_TRUE(handlers.canProcess(ionet::PacketType::Finalization_Proof_At_Height));
		EXPECT_TRUE(handlers.canProcess(ionet::PacketType::Push_Finalization_Messages));
		EXPECT_TRUE(handlers.canProcess(ionet::PacketType::Pull_Finalization_Messages));
	}

	TEST(TEST_CLASS, PacketHandlersAreRegistered_WithVotingDisabled) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot(false);
		const auto& handlers = context.testState().state().packetHandlers();

		// Assert:
		EXPECT_EQ(3u, handlers.size());
		EXPECT_TRUE(handlers.canProcess(ionet::PacketType::Finalization_Statistics));
		EXPECT_TRUE(handlers.canProcess(ionet::PacketType::Finalization_Proof_At_Point));
		EXPECT_TRUE(handlers.canProcess(ionet::PacketType::Finalization_Proof_At_Height));
	}

	// endregion
}}
