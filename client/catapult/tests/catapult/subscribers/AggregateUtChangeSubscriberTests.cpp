#include "catapult/subscribers/AggregateUtChangeSubscriber.h"
#include "tests/catapult/subscribers/test/AggregateSubscriberTestContext.h"
#include "tests/catapult/subscribers/test/UnsupportedSubscribers.h"
#include "tests/test/core/TransactionInfoTestUtils.h"
#include "tests/TestHarness.h"

namespace catapult { namespace subscribers {

#define TEST_CLASS AggregateUtChangeSubscriberTests

	namespace {
		using UnsupportedUtChangeSubscriber = test::UnsupportedUtChangeSubscriber<test::UnsupportedFlushBehavior::Throw>;

		template<typename TUtChangeSubscriber>
		using TestContext = test::AggregateSubscriberTestContext<TUtChangeSubscriber, AggregateUtChangeSubscriber<TUtChangeSubscriber>>;
	}

	TEST(TEST_CLASS, NotifyAddsForwardsToAllSubscribers) {
		// Arrange:
		DEFINE_MOCK_INFOS_CAPTURE(UtChangeSubscriber, notifyAdds);

		TestContext<MockUtChangeSubscriber> context;
		auto transactionInfos = test::CopyTransactionInfosToSet(test::CreateTransactionInfos(3));

		// Sanity:
		EXPECT_EQ(3u, context.subscribers().size());

		// Act:
		context.aggregate().notifyAdds(transactionInfos);

		// Assert:
		test::AssertInfosDelegation(context, transactionInfos);
	}

	TEST(TEST_CLASS, NotifyRemovesForwardsToAllSubscribers) {
		// Arrange:
		DEFINE_MOCK_INFOS_CAPTURE(UtChangeSubscriber, notifyRemoves);

		TestContext<MockUtChangeSubscriber> context;
		auto transactionInfos = test::CopyTransactionInfosToSet(test::CreateTransactionInfos(3));

		// Sanity:
		EXPECT_EQ(3u, context.subscribers().size());

		// Act:
		context.aggregate().notifyRemoves(transactionInfos);

		// Assert:
		test::AssertInfosDelegation(context, transactionInfos);
	}

	TEST(TEST_CLASS, FlushForwardsToAllSubscribers) {
		// Arrange:
		DEFINE_MOCK_FLUSH_CAPTURE(UtChangeSubscriber);

		TestContext<MockUtChangeSubscriber> context;

		// Sanity:
		EXPECT_EQ(3u, context.subscribers().size());

		// Act:
		context.aggregate().flush();

		// Assert:
		test::AssertFlushDelegation(context);
	}
}}
