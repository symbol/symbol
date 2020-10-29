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

#include "syncsource/src/SyncSourceService.h"
#include "tests/test/core/PacketTestUtils.h"
#include "tests/test/core/mocks/MockTransaction.h"
#include "tests/test/local/ServiceLocatorTestContext.h"
#include "tests/test/local/ServiceTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace syncsource {

#define TEST_CLASS SyncSourceServiceTests

	namespace {
		struct SyncSourceServiceTraits {
			static constexpr auto CreateRegistrar = CreateSyncSourceServiceRegistrar;
		};

		class TestContext : public test::ServiceLocatorTestContext<SyncSourceServiceTraits> {
		public:
			explicit TestContext(bool isChainSynced = true) : m_numPushedBlockElements(0) {
				// set up hooks (only increment the counters if the input source is correct)
				auto& hooks = testState().state().hooks();
				hooks.setChainSyncedPredicate([isChainSynced]() { return isChainSynced; });
				hooks.setBlockRangeConsumerFactory([&counter = m_numPushedBlockElements](auto source) {
					return [&counter, source](auto&&) { counter += disruptor::InputSource::Remote_Push == source ? 1 : 0; };
				});

				hooks.setLocalFinalizedHeightHashPairSupplier([]() { return model::HeightHashPair{ Height(1), Hash256() }; });

				// the service needs to be able to parse the mock transactions sent to it
				testState().pluginManager().addTransactionSupport(mocks::CreateMockTransactionPlugin());
			}

		public:
			size_t numPushedBlockElements() {
				return m_numPushedBlockElements;
			}

		private:
			size_t m_numPushedBlockElements;
		};
	}

	// region basic

	ADD_SERVICE_REGISTRAR_INFO_TEST(SyncSource, Post_Range_Consumers)

	TEST(TEST_CLASS, NoServicesOrCountersAreRegistered) {
		test::AssertNoServicesOrCountersAreRegistered<TestContext>();
	}

	TEST(TEST_CLASS, PacketHandlersAreRegistered) {
		// Arrange:
		TestContext context;

		// Act:
		context.boot();
		const auto& handlers = context.testState().state().packetHandlers();

		// Assert:
		EXPECT_EQ(6u, handlers.size());
		EXPECT_TRUE(handlers.canProcess(ionet::PacketType::Push_Block));
		EXPECT_TRUE(handlers.canProcess(ionet::PacketType::Pull_Block));

		EXPECT_TRUE(handlers.canProcess(ionet::PacketType::Chain_Statistics));
		EXPECT_TRUE(handlers.canProcess(ionet::PacketType::Block_Hashes));
		EXPECT_TRUE(handlers.canProcess(ionet::PacketType::Pull_Blocks));

		EXPECT_TRUE(handlers.canProcess(ionet::PacketType::Pull_Transactions));
	}

	// endregion

	// region hook wiring

	namespace {
		void AssertBlockPush(bool isChainSynced, size_t numExpectedPushes) {
			// Arrange:
			TestContext context(isChainSynced);
			context.boot();

			// Act:
			ionet::ServerPacketHandlerContext handlerContext;
			const auto& handlers = context.testState().state().packetHandlers();
			handlers.process(*test::GenerateRandomBlockPacket(), handlerContext);

			// Assert:
			EXPECT_EQ(numExpectedPushes, context.numPushedBlockElements());
		}
	}

	TEST(TEST_CLASS, CanPushBlockWhenSynced) {
		AssertBlockPush(true, 1);
	}

	TEST(TEST_CLASS, CannotPushBlockWhenNotSynced) {
		AssertBlockPush(false, 0);
	}

	// endregion
}}
