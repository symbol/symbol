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
#include "tests/test/int/LocalNodeTestContext.h"
#include "tests/TestHarness.h"
#include <boost/filesystem.hpp>

namespace catapult { namespace local {

#define TEST_CLASS LocalNodeTests

	namespace {
		using NodeFlag = test::NodeFlag;
		constexpr auto Sentinel_Counter_Value = extensions::ServiceLocator::Sentinel_Counter_Value;

		class TestContext : public test::LocalNodeTestContext<test::LocalNodePeerTraits> {
		public:
			using test::LocalNodeTestContext<test::LocalNodePeerTraits>::LocalNodeTestContext;

		public:
			void waitForNumActiveBroadcastWriters(size_t value) const {
				WAIT_FOR_VALUE_EXPR(value, stats().NumActiveBroadcastWriters);
			}

		public:
			bool hasSavedState() const {
				return boost::filesystem::exists(getSupplementalStatePath());
			}

			Height loadSavedChainHeight() const {
				auto path = getSupplementalStatePath();
				io::RawFile file(path.generic_string(), io::OpenMode::Read_Only);
				file.seek(file.size() - sizeof(Height));
				return io::Read<Height>(file);
			}

		public:
			void assertShutdown() const {
				// Assert:
				auto stats = this->stats();
				EXPECT_EQ(Sentinel_Counter_Value, stats.NumActiveReaders);
				EXPECT_EQ(Sentinel_Counter_Value, stats.NumActiveWriters);
				EXPECT_EQ(Sentinel_Counter_Value, stats.NumScheduledTasks);
				EXPECT_EQ(Sentinel_Counter_Value, stats.NumActiveBroadcastWriters);
			}
		};
	}

	// region basic tests

	TEST(TEST_CLASS, CanBootLocalNodeWithoutPeers) {
		// Assert:
		test::AssertCanBootLocalNodeWithoutPeers<TestContext>([](const auto&, const auto& stats) {
			EXPECT_EQ(0u, stats.NumActiveBroadcastWriters);
		});
	}

	TEST(TEST_CLASS, CanBootLocalNodeWithPeers) {
		// Assert:
		test::AssertCanBootLocalNodeWithPeers<TestContext>([](const auto&, const auto& stats) {
			EXPECT_EQ(0u, stats.NumActiveBroadcastWriters);
		});
	}

	TEST(TEST_CLASS, CanShutdownNode) {
		// Assert:
		test::AssertCanShutdownLocalNode<TestContext>();
	}

	TEST(TEST_CLASS, AllPeriodicTasksAreScheduled) {
		// Assert:
		test::AssertLocalNodeSchedulesTasks<TestContext>(9);
	}

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
		EXPECT_TRUE(test::HasCounter(counters, "UT CACHE")) << "basic local node counters";
		EXPECT_TRUE(test::HasCounter(counters, "MEM CUR RSS")) << "memory counters";
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

	TEST(TEST_CLASS, BootUnlocksAccountIfAutoHarvestIsEnabled) {
		// Assert:
		AssertNumUnlockedAccounts(Auto_Harvest, 1);
	}

	TEST(TEST_CLASS, BootDoesNotUnlockAccountIfAutoHarvestIsDisabled) {
		// Assert:
		AssertNumUnlockedAccounts(!Auto_Harvest, 0);
	}

	// endregion

	// region connection tests

	namespace {
		template<typename THandler>
		void RunExternalConnectionTest(unsigned short port, THandler handler) {
			// Arrange: boot a local node and wait for the node to connect to the peer
			TestContext context(NodeFlag::With_Partner, { test::CreateLocalPartnerNode() });
			context.waitForNumActiveWriters(1);

			// Act: create an external connection to the node
			auto clientConnection = test::CreateExternalConnection(port);
			handler(context);
		}
	}

	TEST(TEST_CLASS, CanConnectToLocalNodeAsReader) {
		// Act:
		RunExternalConnectionTest(test::Local_Node_Port, [](auto& context) {
			context.waitForNumActiveReaders(1);
			auto stats = context.stats();

			// Assert:
			EXPECT_EQ(1u, stats.NumActiveReaders);
			EXPECT_EQ(1u, stats.NumActiveWriters);
			EXPECT_EQ(0u, stats.NumActiveBroadcastWriters);
		});
	}

	TEST(TEST_CLASS, CanConnectToLocalNodeAsBroadcastWriter) {
		// Act:
		RunExternalConnectionTest(test::Local_Node_Api_Port, [](auto& context) {
			context.waitForNumActiveBroadcastWriters(1);
			auto stats = context.stats();

			// Assert:
			EXPECT_EQ(0u, stats.NumActiveReaders);
			EXPECT_EQ(1u, stats.NumActiveWriters);
			EXPECT_EQ(1u, stats.NumActiveBroadcastWriters);
		});
	}

	TEST(TEST_CLASS, CanShutdownLocalNodeWithExternalConnections) {
		// Arrange: boot a local node and wait for the node to connect to the peer
		TestContext context(NodeFlag::With_Partner, { test::CreateLocalPartnerNode() });
		context.waitForNumActiveWriters(1);

		// Act: create external connections to the node
		auto connection1 = test::CreateExternalConnection(test::Local_Node_Port);
		auto connection2 = test::CreateExternalConnection(test::Local_Node_Api_Port);
		context.waitForNumActiveReaders(1);
		context.waitForNumActiveBroadcastWriters(1);

		// Act: shutdown the local node
		CATAPULT_LOG(debug) << "shutting down local node";
		context.localNode().shutdown();

		// Assert: the shutdown completed successfully without hanging
		context.assertShutdown();
	}

	// endregion

	// region state saving

	TEST(TEST_CLASS, ShutdownSavesStateToDiskOnSuccessfulBoot) {
		// Arrange:
		TestContext context(NodeFlag::Regular);

		// Sanity:
		EXPECT_FALSE(context.hasSavedState());

		// Act:
		context.localNode().shutdown();

		// Assert:
		EXPECT_TRUE(context.hasSavedState());
		EXPECT_EQ(Height(1), context.loadSavedChainHeight());
	}

	TEST(TEST_CLASS, ShutdownDoesNotSaveStateToDiskOnFailedBoot) {
		// Arrange: create saved state
		TestContext context(NodeFlag::Regular);
		context.localNode().shutdown();

		// Sanity:
		EXPECT_TRUE(context.hasSavedState());
		EXPECT_EQ(Height(1), context.loadSavedChainHeight());

		// - prepare bad config
		auto badConfig = context.createConfig();
		const_cast<model::BlockChainConfiguration&>(badConfig.BlockChain).Plugins.emplace(
				"catapult.plugins.awesome",
				utils::ConfigurationBag({}));

		// Act + Assert: simulate a boot failure by specifying an incorrect plugin
		EXPECT_THROW(context.boot(std::move(badConfig)), catapult_runtime_error);

		// Assert: the config was not overwritten
		EXPECT_TRUE(context.hasSavedState());
		EXPECT_EQ(Height(1), context.loadSavedChainHeight());
	}

	// endregion
}}
