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

#include "finalization/src/FinalizationService.h"
#include "finalization/tests/test/FinalizationBootstrapperServiceTestUtils.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/test/local/PacketWritersServiceTestUtils.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/test/net/RemoteAcceptServer.h"
#include "tests/test/net/mocks/MockPacketWriters.h"
#include "tests/TestHarness.h"

namespace catapult { namespace finalization {

#define TEST_CLASS FinalizationServiceTests

	namespace {
		constexpr auto Num_Dependent_Services = test::FinalizationBootstrapperServiceTestUtils::Num_Bootstrapper_Services;

		struct FinalizationServiceTraits {
			static constexpr auto Counter_Name = "FIN WRITERS";
			static constexpr auto Num_Expected_Services = 1 + Num_Dependent_Services; // writers (1) + dependent services

			static auto GetWriters(const extensions::ServiceLocator& locator) {
				return locator.service<net::PacketWriters>("fin.writers");
			}

			static auto CreateRegistrar(bool enableVoting) {
				auto config = FinalizationConfiguration::Uninitialized();
				config.EnableVoting = enableVoting;
				return CreateFinalizationServiceRegistrar(config);
			}

			static auto CreateRegistrar() {
				return CreateRegistrar(true);
			}
		};

		using TestContext = test::MessageRangeConsumerDependentServiceLocatorTestContext<FinalizationServiceTraits>;

		struct Mixin {
			using TraitsType = FinalizationServiceTraits;
			using TestContextType = TestContext;
		};
	}

	ADD_SERVICE_REGISTRAR_INFO_TEST(Finalization, Post_Extended_Range_Consumers)

	ADD_PACKET_WRITERS_SERVICE_TEST(TEST_CLASS, Mixin, CanBootService)
	ADD_PACKET_WRITERS_SERVICE_TEST(TEST_CLASS, Mixin, CanShutdownService)
	ADD_PACKET_WRITERS_SERVICE_TEST(TEST_CLASS, Mixin, CanConnectToExternalServer)
	ADD_PACKET_WRITERS_SERVICE_TEST(TEST_CLASS, Mixin, WritersAreRegisteredInBannedNodeIdentitySink)

	// region packetIoPickers

	TEST(TEST_CLASS, WritersAreRegisteredInPacketIoPickers) {
		// Arrange: create a (tcp) server
		test::RemoteAcceptServer server;
		server.start();

		// Act: create and boot the service
		TestContext context;
		context.boot();
		auto pickers = context.testState().state().packetIoPickers();

		// - get the packet writers and attempt to connect to the server
		test::ConnectToLocalHost(*FinalizationServiceTraits::GetWriters(context.locator()), server.caPublicKey());

		// Assert: the writers are registered with role `Voting`
		EXPECT_EQ(0u, pickers.pickMatching(utils::TimeSpan::FromSeconds(1), ionet::NodeRoles::Peer).size());
		EXPECT_EQ(1u, pickers.pickMatching(utils::TimeSpan::FromSeconds(1), ionet::NodeRoles::Voting).size());
	}

	// endregion

	// region tasks

	TEST(TEST_CLASS, TasksAreRegistered_WithVotingEnabled) {
		// Arrange:
		TestContext context;
		context.boot(true);

		// Act + Assert:
		test::AssertRegisteredTasksPostBoot(context, {
			"connect peers task for service Finalization",
			"pull finalization proof task",
			"pull finalization messages task"
		});
	}

	TEST(TEST_CLASS, TasksAreRegistered_WithVotingDisabled) {
		// Arrange:
		TestContext context;
		context.boot(false);

		// Act + Assert:
		test::AssertRegisteredTasksPostBoot(context, {
			"connect peers task for service Finalization",
			"pull finalization proof task"
		});
	}

	// endregion
}}
