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

#include "tests/int/node/test/LocalNodeRequestTestUtils.h"
#include "tests/int/node/test/LocalNodeTestContext.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS LocalNodeStatePersistenceTests

	namespace {
		using NodeFlag = test::NodeFlag;

		// not using PeerLocalNodeTestContext because ShutdownDoesNotSaveStateToDiskOnFailedBoot
		// requires functions only in  LocalNodeTestContext
		class TestContext : public test::LocalNodeTestContext<test::LocalNodePeerTraits> {
		public:
			explicit TestContext(NodeFlag nodeFlag)
					: test::LocalNodeTestContext<test::LocalNodePeerTraits>(nodeFlag | NodeFlag::With_Partner, {})
			{}
		};

		void PushAndWaitForSecondBlock(const TestContext& context) {
			// push and wait for a block with height 2
			test::ExternalSourceConnection connection1(context.publicKey());
			auto pIo = test::PushValidBlock(connection1);

			test::ExternalSourceConnection connection2(context.publicKey());
			test::WaitForLocalNodeHeight(connection2, Height(2));
		}
	}

	// region first boot

	TEST(TEST_CLASS, FirstBootSavesNemesisStateToDiskWhenCacheDatabaseStorageDisabled) {
		// Act:
		TestContext context(NodeFlag::Regular);

		// Assert:
		EXPECT_EQ(Height(1), context.loadSavedStateChainHeight());
	}

	TEST(TEST_CLASS, FirstBootSavesNemesisStateToDiskWhenCacheDatabaseStorageEnabled) {
		// Act:
		TestContext context(NodeFlag::Cache_Database_Storage);

		// Assert:
		EXPECT_EQ(Height(1), context.loadSavedStateChainHeight());
	}

	// endregion

	// region sync + shutdown

	TEST(TEST_CLASS, ShutdownSavesStateToDiskOnSuccessfulBootWhenCacheDatabaseStorageDisabled) {
		// Arrange:
		TestContext context(NodeFlag::Regular);

		// Sanity:
		EXPECT_EQ(Height(1), context.loadSavedStateChainHeight());

		// Act:
		PushAndWaitForSecondBlock(context);

		// Assert: state was not updated
		EXPECT_EQ(Height(1), context.loadSavedStateChainHeight());

		// Act:
		context.localNode().shutdown();

		// Assert: state was updated
		EXPECT_EQ(Height(2), context.loadSavedStateChainHeight());
	}

	TEST(TEST_CLASS, ShutdownSavesStateToDiskOnSuccessfulBootWhenCacheDatabaseStorageEnabled) {
		// Arrange:
		TestContext context(NodeFlag::Cache_Database_Storage);

		// Sanity:
		EXPECT_EQ(Height(1), context.loadSavedStateChainHeight());

		// Act:
		PushAndWaitForSecondBlock(context);

		// Assert: state was updated
		EXPECT_EQ(Height(2), context.loadSavedStateChainHeight());

		// Act:
		context.localNode().shutdown();

		// Assert: state didn't change
		EXPECT_EQ(Height(2), context.loadSavedStateChainHeight());
	}

	// endregion

	// region boot failure

	TEST(TEST_CLASS, ShutdownDoesNotSaveStateToDiskOnFailedBoot) {
		// Arrange: create saved state
		TestContext context(NodeFlag::Regular);
		PushAndWaitForSecondBlock(context);
		context.localNode().shutdown();

		// Sanity:
		EXPECT_EQ(Height(2), context.loadSavedStateChainHeight());

		// - prepare bad config
		auto badConfig = context.createConfig();
		const_cast<model::BlockChainConfiguration&>(badConfig.BlockChain).Plugins.emplace(
				"catapult.plugins.awesome",
				utils::ConfigurationBag({}));

		// Act + Assert: simulate a boot failure by specifying an incorrect plugin
		EXPECT_THROW(context.boot(std::move(badConfig)), catapult_runtime_error);

		// Assert: the config was not overwritten
		EXPECT_EQ(Height(2), context.loadSavedStateChainHeight());
	}

	// endregion
}}
