#include "catapult/subscribers/AggregateNodeSubscriber.h"
#include "catapult/ionet/Node.h"
#include "tests/catapult/subscribers/test/AggregateSubscriberTestContext.h"
#include "tests/test/other/mocks/MockNodeSubscriber.h"
#include "tests/TestHarness.h"

namespace catapult { namespace subscribers {

#define TEST_CLASS AggregateNodeSubscriberTests

	namespace {
		template<typename TNodeSubscriber>
		using TestContext = test::AggregateSubscriberTestContext<TNodeSubscriber, AggregateNodeSubscriber<TNodeSubscriber>>;
	}

	TEST(TEST_CLASS, NotifyNodeForwardsToAllSubscribers) {
		// Arrange:
		TestContext<mocks::MockNodeSubscriber> context;
		auto node = ionet::Node();

		// Sanity:
		EXPECT_EQ(3u, context.subscribers().size());

		// Act:
		context.aggregate().notifyNode(node);

		// Assert:
		auto i = 0u;
		for (const auto* pSubscriber : context.subscribers()) {
			auto message = "subscriber at " + std::to_string(i++);
			const auto& capturedParams = pSubscriber->params();
			ASSERT_EQ(1u, capturedParams.size()) << message;
			EXPECT_EQ(&node, &capturedParams[0].Node) << message;
		}
	}
}}
