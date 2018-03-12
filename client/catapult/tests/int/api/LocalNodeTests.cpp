#include "catapult/extensions/ServiceLocator.h"
#include "tests/test/int/LocalNodeTestContext.h"
#include "tests/TestHarness.h"

namespace catapult { namespace local {

#define TEST_CLASS LocalNodeTests

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

			void assertStateLoaded() const {
				EXPECT_EQ(1u, numPreLoadHandlerCalls());
			}
		};
	}

	// region basic tests

	TEST(TEST_CLASS, CanBootLocalNodeWithoutPeers) {
		// Assert:
		test::AssertCanBootLocalNodeWithoutPeers<TestContext>([](const auto& context, const auto&) {
			context.assertStateLoaded();
		});
	}

	TEST(TEST_CLASS, CanBootLocalNodeWithPeers) {
		// Assert:
		test::AssertCanBootLocalNodeWithPeers<TestContext>([](const auto& context, const auto&) {
			context.assertStateLoaded();
		});
	}

	TEST(TEST_CLASS, CanShutdownNode) {
		// Assert:
		test::AssertCanShutdownLocalNode<TestContext>();
	}

	TEST(TEST_CLASS, AllPeriodicTasksAreScheduled) {
		// Assert:
		test::AssertLocalNodeSchedulesTasks<TestContext>(7);
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
		EXPECT_TRUE(test::HasCounter(counters, "UT CACHE")) << "basic local node counters";
		EXPECT_TRUE(test::HasCounter(counters, "MEM CUR RSS")) << "memory counters";
	}

	// endregion

	// region connection tests

	TEST(TEST_CLASS, CannotConnectToApiPort) {
		// Assert:
		test::AssertConnectionError<TestContext>(test::Local_Node_Api_Port);
	}

	namespace {
		template<typename THandler>
		void RunExternalReaderTest(THandler handler) {
			// Arrange: boot a partner node
			TestContext context(NodeFlag::With_Partner, { test::CreateLocalPartnerNode() });
			context.waitForNumActiveWriters(1);

			// Act: create an external connection to the node
			auto clientConnection = test::CreateExternalConnection(test::Local_Node_Port);
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
