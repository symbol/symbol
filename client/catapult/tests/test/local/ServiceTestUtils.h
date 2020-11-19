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
#include "catapult/extensions/ServiceRegistrar.h"
#include "catapult/ionet/PacketType.h"
#include "tests/TestHarness.h"

namespace catapult { namespace test {

	/// Asserts that the service registrar produces correct information (\a expectedName, \a expectedPhase).
	template<typename TTraits>
	void AssertServiceRegistrarSelfReportsCorrectInformation(
			const std::string& expectedName,
			extensions::ServiceRegistrarPhase expectedPhase) {
		// Arrange:
		auto pService = TTraits::CreateRegistrar();

		// Act:
		auto info = pService->info();

		// Assert:
		EXPECT_EQ(expectedName, info.Name);
		EXPECT_EQ(expectedPhase, info.Phase);
	}

/// Adds a test for ServiceRegistrar::info given a service name (\a SERVICE_NAME) and expected phase (\a EXPECTED_PHASE).
#define ADD_SERVICE_REGISTRAR_INFO_TEST(SERVICE_NAME, EXPECTED_PHASE) \
	TEST(SERVICE_NAME##ServiceTests, ServiceRegistrarSelfReportsCorrectInformation) { \
		test::AssertServiceRegistrarSelfReportsCorrectInformation<SERVICE_NAME##ServiceTraits>( \
				#SERVICE_NAME, \
				extensions::ServiceRegistrarPhase::EXPECTED_PHASE); \
	}

	/// Asserts that the service does not register any services or counters.
	template<typename TTestContext>
	void AssertNoServicesOrCountersAreRegistered() {
		// Arrange:
		TTestContext context;

		// Act:
		context.boot();

		// Assert:
		EXPECT_EQ(0u, context.locator().numServices());
		EXPECT_EQ(0u, context.locator().counters().size());
	}

	/// Asserts that the service registers a single packet handler for \a packetType.
	template<typename TTestContext>
	void AssertSinglePacketHandlerIsRegistered(ionet::PacketType packetType) {
		// Arrange:
		TTestContext context;

		// Act:
		context.boot();
		const auto& handlers = context.testState().state().packetHandlers();

		// Assert:
		EXPECT_EQ(1u, handlers.size());
		EXPECT_TRUE(handlers.canProcess(packetType));
	}
}}
