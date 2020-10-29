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

#include "catapult/extensions/ServiceLocator.h"
#include "tests/int/node/test/LocalNodeBasicTests.h"
#include "tests/int/node/test/LocalNodeTestContext.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS LocalNodeBasicTests

	namespace {
		using NodeFlag = test::NodeFlag;
		constexpr auto Sentinel_Counter_Value = extensions::ServiceLocator::Sentinel_Counter_Value;

		class TestContext : public test::LocalNodeTestContext<test::LocalNodePeerTraits> {
		public:
			using test::LocalNodeTestContext<test::LocalNodePeerTraits>::LocalNodeTestContext;

		public:
			void assertShutdown() const {
				// Assert:
				auto stats = this->stats();
				EXPECT_EQ(Sentinel_Counter_Value, stats.NumActiveReaders);
				EXPECT_EQ(Sentinel_Counter_Value, stats.NumActiveWriters);
				EXPECT_EQ(Sentinel_Counter_Value, stats.NumScheduledTasks);
			}
		};

		struct BasicTestContext {
			using LocalNodeTestContext = TestContext;

			static constexpr auto Num_Tasks = 10u;

			static void AssertBoot(const test::PeerLocalNodeStats&)
			{}
		};
	}

	// region basic tests

	DEFINE_LOCAL_NODE_BASIC_TESTS(BasicTestContext)

	TEST(TEST_CLASS, AllCounterGroupsAreRegistered) {
		// Act:
		TestContext context(NodeFlag::Regular);

		const auto& counters = context.localNode().counters();
		for (const auto& counter : counters)
			CATAPULT_LOG(debug) << "counter: " << counter.id().name();

		// Assert: check candidate counters
		EXPECT_TRUE(test::HasCounter(counters, "ACNTST C")) << "cache counters";
		EXPECT_TRUE(test::HasCounter(counters, "TX ELEM TOT")) << "service local node counters";
		EXPECT_TRUE(test::HasCounter(counters, "UNLKED ACCTS")) << "peer local node counters";
		EXPECT_TRUE(test::HasCounter(counters, "UT CACHE")) << "local node counters";
		EXPECT_TRUE(test::HasCounter(counters, "TOT CONF TXES")) << "local node counters";
		EXPECT_TRUE(test::HasCounter(counters, "MEM CUR RSS")) << "memory counters";
		EXPECT_TRUE(test::HasCounter(counters, "NODES")) << "node container counters";
		EXPECT_TRUE(test::HasCounter(counters, "BAN ACT")) << "banned nodes container counters";
		EXPECT_TRUE(test::HasCounter(counters, "BAN ALL")) << "banned nodes container counters";
	}

	// endregion

	// region auto harvest tests

	namespace {
		constexpr bool Auto_Harvest = true;

		void AssertNumUnlockedAccounts(bool shouldAutoHarvest, size_t expectedNumUnlockedAccounts) {
			// Arrange:
			auto flags = shouldAutoHarvest ? NodeFlag::Auto_Harvest : NodeFlag::Regular;

			// Act
			TestContext context(flags);
			context.waitForNumActiveReaders(0);
			auto stats = context.stats();

			// Assert:
			EXPECT_EQ(expectedNumUnlockedAccounts, stats.NumUnlockedAccounts);
		}
	}

	TEST(TEST_CLASS, BootUnlocksAccountWhenAutoHarvestIsEnabled) {
		AssertNumUnlockedAccounts(Auto_Harvest, 1);
	}

	TEST(TEST_CLASS, BootDoesNotUnlockAccountWhenAutoHarvestIsDisabled) {
		AssertNumUnlockedAccounts(!Auto_Harvest, 0);
	}

	// endregion

	// region connection tests

	namespace {
		template<typename THandler>
		void RunExternalConnectionTest(unsigned short port, THandler handler) {
			// Arrange: boot a local node and wait for the node to connect to the peer
			TestContext context(NodeFlag::With_Partner, {});
			context.waitForNumActiveWriters(1);

			// Act: create an external connection to the node
			auto clientConnection = test::CreateExternalConnection(port);
			handler(context);
		}
	}

	TEST(TEST_CLASS, CanConnectToLocalNodeAsReader) {
		// Act:
		RunExternalConnectionTest(test::GetLocalNodePort(), [](auto& context) {
			context.waitForNumActiveReaders(1);
			auto stats = context.stats();

			// Assert:
			EXPECT_EQ(1u, stats.NumActiveReaders);
			EXPECT_EQ(1u, stats.NumActiveWriters);
		});
	}

	TEST(TEST_CLASS, CanShutdownLocalNodeWithExternalConnections) {
		// Arrange: boot a local node and wait for the node to connect to the peer
		TestContext context(NodeFlag::With_Partner, {});
		context.waitForNumActiveWriters(1);

		// Act: create an external connection to the node
		auto connection = test::CreateExternalConnection(test::GetLocalNodePort());
		context.waitForNumActiveReaders(1);

		// Act: shutdown the local node
		CATAPULT_LOG(debug) << "shutting down local node";
		context.localNode().shutdown();

		// Assert: the shutdown completed successfully without hanging
		context.assertShutdown();
	}

	// endregion
}}
