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

#define TEST_CLASS LocalNodeApiBasicTests

	namespace {
		using NodeFlag = test::NodeFlag;
		constexpr auto Sentinel_Counter_Value = extensions::ServiceLocator::Sentinel_Counter_Value;

		class TestContext : public test::LocalNodeTestContext<test::LocalNodeApiTraits> {
		public:
			using test::LocalNodeTestContext<test::LocalNodeApiTraits>::LocalNodeTestContext;

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

			static constexpr auto Num_Tasks = 9u;

			static void AssertBoot(const test::BasicLocalNodeStats&)
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
		EXPECT_TRUE(test::HasCounter(counters, "UT CACHE")) << "local node counters";
		EXPECT_TRUE(test::HasCounter(counters, "TOT CONF TXES")) << "local node counters";
		EXPECT_TRUE(test::HasCounter(counters, "MEM CUR RSS")) << "memory counters";
		EXPECT_TRUE(test::HasCounter(counters, "NODES")) << "node container counters";
		EXPECT_TRUE(test::HasCounter(counters, "BAN ACT")) << "banned nodes container counters";
		EXPECT_TRUE(test::HasCounter(counters, "BAN ALL")) << "banned nodes container counters";
	}

	// endregion

	// region connection tests

	namespace {
		template<typename THandler>
		void RunExternalReaderTest(THandler handler) {
			// Arrange: boot a partner node
			TestContext context(NodeFlag::With_Partner, {});
			context.waitForNumActiveWriters(1);

			// Act: create an external connection to the node
			auto clientConnection = test::CreateExternalConnection(test::GetLocalNodePort());
			context.waitForNumActiveReaders(1);
			handler(context);
		}
	}

	TEST(TEST_CLASS, CanConnectToLocalNodeAsReader) {
		// Act:
		RunExternalReaderTest([](auto& context) {
			auto stats = context.stats();

			// Assert:
			EXPECT_EQ(1u, stats.NumActiveReaders);
			EXPECT_EQ(1u, stats.NumActiveWriters);
		});
	}

	TEST(TEST_CLASS, CanShutdownLocalNodeWithExternalConnections) {
		// Act:
		RunExternalReaderTest([](auto& context) {
			// Act: shutdown the local node
			CATAPULT_LOG(debug) << "shutting down local node";
			context.localNode().shutdown();

			// Assert: the shutdown completed successfully without hanging
			context.assertShutdown();
		});
	}

	// endregion
}}
